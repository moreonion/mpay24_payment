<?php

module_load_include('inc', 'mpay24_payment', 'mpay24_payment_method_controller_common');
module_load_include('inc', 'mpay24_payment', 'mpay24.api');

/**
 * @file
 * MPay24 payment method controller for credit card payments
 */
class PaymentMethodControllerMPay24CreditCard extends PaymentMethodControllerMPay24 {
  public $payment_configuration_form_elements_callback = 'mpay24_payment_payment_configuration_form_elements';

  function __construct() {
    parent::__construct();

    $this->title = t('mPay24 Credit Card');
  }


  /**
   * Implements PaymentMethodController::validate().
   */
  function validate(Payment $payment, PaymentMethod $payment_method, $strict) {
  }


  /**
   * Implements PaymentMethodController::execute().
   *
   * Redirect to a site which polls the status of mPay24s confirmation interface from
   * our server and set the payment status accordingly
   */
  function execute(Payment $payment) {
    $response = new \mpay24\Response($payment, \mpay24\Request::creditCard($payment));
    drupal_goto($response->uri);
  }
}

/**
 * Implements
 * PaymentMethodController::payment_configuration_form_elements_callback.
 */
function mpay24_payment_payment_configuration_form_elements(array $element, array &$form_state) {
  $element = array(
    '#type'         => 'fieldset',
    '#collapsible'	=> TRUE,
    '#collapsed'	=> TRUE,
    '#title'		=> t('Credit Card')
  );

  $issuers = array(
    1 => 'Visa',
    2 => 'MasterCard',
    3 => 'American Express',
  );

  $element['issuer'] = array(
    '#type'		    => 'select',
    '#options'   	=> $issuers,
    '#empty_value'	=> '',
    '#title'		=> t('Issuer'),
    '#weight'		=> 0,
    );

  $element['credit_card_number'] = array(
    '#type'      => 'textfield',
    '#title'     => t('Credit card number '),
    '#weight'    => 1,
    '#size'      => 32,
    '#maxlength' => 32,
  );

  $element['secure_code'] = array(
    '#type'      => 'textfield',
    '#title'     => t('Secure code'),
    '#weight'    => 2,
    '#size'      => 4,
    '#maxlength' => 4,
  );

  $element['expiry_date'] = array(
    '#type'      => 'textfield',
    '#title'     => t('Expiry date'),
    '#weight'    => 3,
    '#size'      => 5,
    '#maxlength' => 5,
  );



  return $element;
}

/**
 * Implements form validate callback for
 * mpay24_payment_payment_configuration_form_elements
 */
function mpay24_payment_payment_configuration_form_elements_validate(array $element, array &$form_state) {
  $values = drupal_array_get_nested_value($form_state['values'], $element['#parents']);

  # validate presence of all fields
  foreach($values as $key => $value) {
    if(empty($value)) {
      form_error($element[$key], t('@name is required', array('@name' => $element[$key]['#title'])));
    }

    if ($element[$key]['#type'] === 'select') {
      $form_state['payment']->method_data[$key] = $element[$key]['#options'][$value];
    } else {
      $form_state['payment']->method_data[$key] = $value;
    }
  }

  require_once('creditcard_validation.inc.php');

  $credit_card_validator = new CreditCardValidator();

  # map donation form values to cc issuers
  $issuers = array(
    1 => 'visa',
    2 => 'mastercard',
    3 => 'amex',
  );

  $issuer             = $element['issuer']['#value'];
  $credit_card_number = $element['credit_card_number']['#value'];
  $secure_code        = $element['secure_code']['#value'];
  $expiry_date        = $element['expiry_date']['#value'];

  # validate creditcard number
  $validation_result = $credit_card_validator->isValidCreditCard($credit_card_number, '', TRUE);
  if ($validation_result->valid == FALSE) {
    form_error($element['credit_card_number'], t('%card is not a valid credit card number.', array('%card' => $credit_card_number)));
  }
  elseif ($validation_result->issuer != $issuers[$issuer]) {
    form_error($element['credit_card_number'], t(
      'Credit card number %card doesn\'t appear to be from issuer %issuer.',
      array(
        '%card'   => $credit_card_number,
        '%issuer' => $issuers[$issuer]
      )
    )
  );
  }

  # validate secure code (CVC)
  if ($credit_card_validator->isValidCardValidationCode($secure_code, $issuers[$issuer]) == FALSE) {
    switch ($issuers[$issuer]) {
    case 'visa':
      $cvc_labeling = 'CVV2 (Card Verification Value 2)';
      break;

    case 'mastercard':
      $cvc_labeling = 'CVC2 (Card Validation Code 2)';
      break;

    case 'amex':
      $cvc_labeling = 'CID (Card Identification Number)';
      break;

    default:
      $cvc_labeling = 'CVC2 (Card Validation Code 2)';
    }
    form_error($element['secure_code'], t('The ' . $cvc_labeling . ' %card is not valid.', array('%card' => $secure_code)));
  }

  # validate expiry date
  if ($credit_card_validator->isValidToDate($expiry_date) == FALSE) {
    form_error($element['expiry_date'], t('The entered expiration date is wrong or the card is expired.'));
  }

}
