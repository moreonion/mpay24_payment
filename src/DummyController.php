<?php

namespace Drupal\mpay24_payment;

class DummyController extends  ControllerBase {

  public $payment_method_configuration_form_elements_callback = 'payment_forms_method_configuration_form';
  public $payment_configuration_form_elements_callback = 'payment_forms_payment_form';

  public function __construct() {
    parent::__construct();
    $this->title = 'Mpay24 Dummy';
    $this->description = 'This payment method allows to mock server-side behavior.';
  }
  
  public function paymentForm() {
    return new DummyPaymentForm();
  }

  public function configurationForm() {
    return new DummyConfigurationForm();
  }

  /**
   * Implements PaymentMethodController::validate().
   */
  function validate(\Payment $payment, \PaymentMethod $payment_method, $strict) {
    if (!$strict)
      return;

    sleep($payment->context_data['method_data']['validate_timeout']);
  }
  
  /**
   * Implements PaymentMethodController::execute().
   */
  function execute(\Payment $payment) {
    $request = new \mpay24\Request($payment);

    $data = &$payment->context_data['method_data'];
    $options['query']['TID'] = $payment->tid;
    $redirect = NULL;
    switch ($data['action']) {
      case 'error':
        echo 'Some test output';
        exit;
      case 'poll':
        $payment->setStatus(new \PaymentStatusItem(PAYMENT_MPAY24_STATUS_REDIRECT));
        $path = MPAY24_PAYMENT_CONFIRMATION_WAIT_URL . (int) $payment->pid;
        $redirect = array($path, $options);
        break;
      case 'error_page':
        $redirect = array($request->getErrorURL(), $options);
        break;
      case 'success_page':
      default:
        $redirect = array($request->getSuccessURL(), $options);;
    }
    if ($redirect) {
      $payment->contextObj->redirect($redirect[0], $redirect[1]);
    }
  }
  
  /**
   * Set the target status once the timeout has run out.
   */
  public function pollStatus(\Payment $payment) {
    $status = $payment->getStatus();
    $data = &$payment->context_data['method_data']['poll'];
    if ($status->created + $data['timeout'] < REQUEST_TIME && $status->status != $data['status']) {
      $payment->setStatus(new PaymentStatusItem($data['status'], time(), $payment->pid));
      entity_save('payment', $payment);
    }
  }
}
