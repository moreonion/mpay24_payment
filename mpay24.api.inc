<?php
/**
* @file
* Provides a class to make communication with mpay24 services easier.
*
*/

namespace mpay24;

define('MPAY24_PAYMENT_TEST_API_URL', 'https://test.mpay24.com/app/bin/etpv5');
define('MPAY24_PAYMENT_API_URL', 'https://mpay24.com/app/bin/etpv5');

class PaymentMpay24ApiException extends \PaymentException {
  protected $errorCode;
  public function __construct($message, $code) {
    parent::__construct($message);
    $this->errorCode = $code;
  }

  public function getErrorCode() {
    return $this->errorCode;
  }
}

class Request {
    /**
     * Constructs a new generic Request.
     * Tipp: Call one of the static methods instead of using this directly.
     */
    function __construct(&$payment) {
        $this->payment = $payment;
        $this->data = array();
        $this->payment_types = array(
          'CB', 'CC', 'ELV', 'EPS', 'GIROPAY', 'HP', 'INVOICE', 'MAESTRO',
          'MPASS', 'PAYPAL', 'PB', 'PSC', 'QUICK', 'SAFETYPAY');


        if($this->getTestMode()) {
            $this->api_url = MPAY24_PAYMENT_TEST_API_URL;
        } else {
            $this->api_url = MPAY24_PAYMENT_API_URL;
        }

        $this->payment->tid = $this->generateTID(time());

        // we need a valid pid for the next step.
        entity_save('payment', $payment);
    }

    public static function creditCard(&$payment) {
        $obj = new Request($payment);
        $method_data = &$payment->method_data;
        // a guard to prevent the later ->format() to fail
        $expiry_date = $method_data['expiry_date'];
        if (get_class($expiry_date) != "DateTime") {
          $expiry_date = date_create();
        }

        if (in_array($method_data['issuer'], array('visa', 'mastercard'))) {
          $auth_3ds = TRUE;
        } else {
          $auth_3ds = (bool) $method_data['auth_3ds'];
        }

        $obj->prepare('ACCEPTPAYMENT');
        $obj->addData(array(
            'P_TYPE'     => 'CC',
            'BRAND'      => $method_data['issuer'],
            'IDENTIFIER' => $method_data['credit_card_number'],
            'EXPIRY'     => $expiry_date->format('ym'),
            'CVC'        => $method_data['secure_code'],
            'AUTH_3DS'   => $auth_3ds ? 'Y' : 'N',
        ));

        return $obj->send();
    }

    public static function onlineBanking(&$payment) {
        $obj = new Request($payment);
        $obj->addData($payment->context_data['context']);
        $obj->enabled_payment_types = array('EPS');
        $obj->prepare('SELECTPAYMENT');

        return $obj->send();
    }

    protected function prepare($operation) {
        $this->addData(array(
            'MERCHANTID' => $this->getMerchantID(),
            'OPERATION'  => $operation,
        ));

        if ($operation === 'SELECTPAYMENT') {
            $mdxi = $this->buildMDXI();
            $this->addData(array('MDXI' => $mdxi));
        } else {
            $this->addData(
                array(
                    'CURRENCY'         => 'EUR',
                    'TID'              => $this->payment->tid,
                    'PRICE'            => $this->payment->totalAmount(false) * 100, // in cent
                    'CUSTOMER'         => $this->getCustomerName(),
                    'SUCCESS_URL'      => $this->getSuccessURL(),
                    'ERROR_URL'        => $this->getErrorURL(),
                    'CONFIRMATION_URL' => $this->getConfirmationURL(),
                )
            );
        }
    }

    protected function send() {
        $options = array(
            'method' => 'POST',
            'data'   => drupal_http_build_query($this->data));
        $this->response = drupal_http_request($this->api_url, $options);
        if (variable_get('devel_mode', 0)) {
          watchdog('mpay24_payment', 'Received response: !response', array('!response' => var_export($this->response, TRUE)), WATCHDOG_DEBUG);
	}
        return $this->response;
    }

    protected function addData($data) {
        $this->data = array_merge($this->data, $data);
    }

    /**
     * Build mpay24s MDXI-XML for this request
     *
     * Only used for payment pages; gets called automatically for in
     * $this->prepare().
     *
     * @return string MXDI
     */

    function buildMDXI() {
        $disabled_types = '';
        foreach ($this->payment_types as $payment_type) {
            if (!in_array($payment_type, $this->enabled_payment_types)) {
                $disabled_types.= '<Payment Type="' . $payment_type . '"/>';
            }
        }

        return '<?xml version="1.0" encoding="UTF-8"?>' .
            '<Order Style="margin: auto; max-width: 600px;">' .
            '  <Tid>' . $this->payment->tid . '</Tid>' .
            '  <PaymentTypes Enable="false">' . $disabled_types . '</PaymentTypes>' .
            '  <ShoppingCart>' .
            '    <Description>' . $this->payment->description . '</Description>' .
            '  </ShoppingCart>' .
            '  <Price>' . $this->payment->totalAmount(FALSE) . '</Price>' .
            '  <BillingAddr Mode="ReadWrite">' .
            '    <Name>' . $this->getCustomerName() . '</Name>' .
            '  </BillingAddr>' .
            '  <URL>' .
            '    <Success>' . $this->getSuccessURL() . '</Success>' .
            '    <Error>' . $this->getErrorURL() . '</Error>' .
            '    <Confirmation>' . $this->getConfirmationURL() . '</Confirmation>' .
            '  </URL>' .
            '</Order>';
    }

    /**
     * Generate a unique transaction id.
     *
     * Gets called by the $this->__construct().
     *
     * @param array $extra_salt Data which should be used to calculate the tid.
     *
     * @return string TID
     */
    protected function generateTID($extra_salt) {
        $components = array(
            $this->payment->description,
            $this->getCustomerName(),
            drupal_get_hash_salt(),
            $extra_salt
        );

        return md5(implode($components));
    }

    protected function getCustomerName() {
      // make sure that this is in the payment data
      return $this->payment->context_data['context']->valueByKey('first_name') .
        $this->payment->context_data['context']->valueByKey('last_name');
    }

    protected function getMerchantID() {
        if (isset($this->payment->method->controller_data['merchantid'])) {
            return $this->payment->method->controller_data['merchantid'];
        } else {
            throw new \Exception(t('The administrators of this site need to set a merchant id before this form becomes usable'));
        }
    }

    protected function getTestMode() {
        if (isset($this->payment->method->controller_data['testmode'])) {
            return $this->payment->method->controller_data['testmode'];
        } else {
            return false;
        }
    }

    protected function buildURL($relative) {
        return url($relative, array('absolute' => TRUE, 'https' => TRUE));
    }

    public function getSuccessURL() {
      return $this->buildURL(MPAY24_PAYMENT_SUCCESS_URL. $this->payment->pid);
    }

    public function getErrorURL() {
      return $this->buildURL(MPAY24_PAYMENT_ERROR_URL . $this->payment->pid);
    }

    public function getConfirmationURL() {
      return $this->buildURL(MPAY24_PAYMENT_CONFIRMATION_URL . $this->payment->pid);
    }
}


class Response {
    public $parsed_data = array();

    public function __construct($payment, $response) {
      $this->payment  = &$payment;
      $this->response = &$response;

      if (variable_get('mpay24_payment_devel', 0) || variable_get('devel_mode', 0)) {
        watchdog('mpay24_payment', 'mpay24 response for payment !pid: !response', array(
          '!pid' => $this->payment->pid,
          '!response' => var_export($this, TRUE),
          ), WATCHDOG_DEBUG);
      }

      if (isset($response->code) && $response->code === '200') {
        parse_str(rtrim($response->data), $this->data);
        $this->data = array_change_key_case($this->data);

        if (!empty($this->data['mpaytid'])) {
          $payment->mpaytid = $this->data['mpaytid'];
        }

        if($this->data['status'] === 'OK') {
          $this->handleReturnCode();
        } else {
          $this->handleAPIError();
        }
      } else {
        $this->handleHTTPError();
      }
    }

    function handleReturnCode() {
      $this->payment->setStatus(new \PaymentStatusItem(PAYMENT_MPAY24_STATUS_REDIRECT));
      entity_save('payment', $this->payment);

      switch($this->data['returncode']) {
        case 'REDIRECT':
          $this->uri = $this->data['location'];
          break;
        case 'OK':
          $path = MPAY24_PAYMENT_CONFIRMATION_WAIT_URL . (int) $this->payment->pid;
          $options['query']['TID'] = $this->payment->tid;
          $this->uri = array($path, $options);
          break;
        default:
          $this->handleAPIError();
      }
      if (variable_get('mpay24_payment_devel', 0) || variable_get('devel_mode', 0)) {
        watchdog('mpay24_payment', 'Redirect paymen !pid to !uri.', array(
          '!pid' => $this->payment->pid,
          '!uri' => $this->uri,
        ), WATCHDOG_DEBUG);
      }
    }

    function handleAPIError() {
      $this->payment->setStatus(new \PaymentStatusItem(PAYMENT_STATUS_FAILED));
      entity_save('payment', $this->payment);
      $message = t(
        '@payment_method payment method encountered some error while contacting the mPay24 server. The code is "@code" and the error message "@error". Pid payment : @pid and pmid method used : @pmid',
        array(
          '@code'            => $this->data['returncode'],
          '@error'           => isset($this->data['external_error']) ? $this->data['external_error']: "",
          '@pid'             => $this->payment->pid,
          '@pmid'            => $this->payment->method->pmid,
          '@payment_method'  => $this->payment->method->title_generic,
        )
      );
      throw new PaymentMpay24ApiException($message, $this->data['returncode']);
    }

    function handleHTTPError() {
      $this->payment->setStatus(new \PaymentStatusItem(PAYMENT_STATUS_FAILED));
      entity_save('payment', $this->payment);
      $message = t(
        '@payment_method payment method encountered some HTTP error while trying to contact the mPay24 server. Error code "@code" and the error message "@error". Pid payment : @pid and pmid method used : @pmid',
        array(
          '@code'           => $this->response->code,
          '@error'          => $this->response->error,
          '@pid'            => $this->payment->pid,
          '@pmid'           => $this->payment->method->pmid,
          '@payment_method' => $this->payment->method->title_generic,
        )
      );
      throw new \PaymentException($message);
    }
}
