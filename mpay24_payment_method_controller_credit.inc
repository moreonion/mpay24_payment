<?php
/**
 * @file
 * MPay24 payment method controller for credit card payments
 */

module_load_include('inc', 'mpay24_payment', 'mpay24.api');
module_load_include('php', 'mpay24_payment', 'lib/MPay24CreditCardForm');

class PaymentMethodControllerMPay24CreditCard extends PaymentMethodControllerMPay24 {
  protected $response = NULL;
  protected $exception = NULL;

  function __construct() {
    parent::__construct();

    $this->title = t('mPay24 Credit Card');
    $this->form = new \Drupal\mpay24_payment\MPay24CreditCardForm();
  }

  /**
   * Implements PaymentMethodController::validate().
   */
  function validate(Payment $payment, PaymentMethod $payment_method, $strict) {
    if (!$strict) {
    	return;
    }
    if (!isset($this->response)) {
      try {
        $this->response = $this->getResponse($payment);
      } catch (PaymentException $e) {
        $this->exception = $e;
      }
    }
    if ($this->exception) {
      throw $this->exception;
    }
  }

  function getResponse($payment) {
    return new \mpay24\Response($payment, \mpay24\Request::creditCard($payment));
  }

  /**
   * Implements PaymentMethodController::execute().
   *
   * Redirect to a site which polls the status of mPay24s confirmation interface from
   * our server and set the payment status accordingly
   */
  function execute(Payment $payment) {
    if ($this->exception) {
      throw $this->exception;
    }
    $payment->form_state['redirect'] = $this->response->uri;
  }
}
