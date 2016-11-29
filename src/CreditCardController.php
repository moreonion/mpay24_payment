<?php
/**
 * @file
 * MPay24 payment method controller for credit card payments
 */

namespace Drupal\mpay24_payment;

class CreditCardController extends ControllerBase {
  protected $response = NULL;
  protected $exception = NULL;

  function __construct() {
    parent::__construct();
    $this->title = t('mPay24 Credit Card');
  }

  public function paymentForm() {
    return new CreditCardForm();
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
    $response = $this->getResponse($payment);
    $link = $response->link;
    $payment->contextObj->redirect($link['path'], $link);
  }
}
