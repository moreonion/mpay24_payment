<?php

module_load_include('inc', 'mpay24_payment', 'mpay24.api');
module_load_include('php', 'payment_forms', 'lib/OnlineBankingForm');

/**
 * @file
 * MPay24 payment method controller for online banking payments.
 */
class PaymentMethodControllerMPay24OnlineBanking extends PaymentMethodControllerMPay24 {
  function __construct() {
    parent::__construct();

    $this->title = t('mPay24 Online Banking');
    $this->form = new \Drupal\payment_forms\OnlineBankingForm();
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
    $response = new \mpay24\Response($payment, \mpay24\Request::onlineBanking($payment));
    $link = $response->link;
    $payment->contextObj->redirect($link['path'], $link);
  }
}
