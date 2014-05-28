<?php

define('MERCHANT_TESTID', '99999');

namespace Drupal\mpay24_payment;

class CreditCardPaymentTest extends \DrupalUnitTestCase {
  public function createCreditCardPayment($issuer, $number, $secureCode, $expiry) {
    $controller = new \PaymentMethodControllerMPay24CreditCard();
    $method = new \PaymentMethod();
    $method->controller = $controller;
    $method->controller_data = array(
      'testmode' => TRUE,
      'merchantid' => MERCHANT_TESTID, 
    );
    $payment = new \Payment();
    $payment->method = $method;
    $payment->setLineItem(new \PaymentLineItem(array(
      'amount' => 5,
      'description' => 'Test payment',
      'name' => 'test',
    )));
    $payment->method_data += array(
      'issuer' => $issuer,
      'credit_card_number' => $number,
      'secure_code' => $secureCode,
      'expiry_date' => $expiry,
    );
    $payment->form_state = array();
    return $payment;
  }
  public function testValidVisa() {
    $now = \date_create();
    $payment = $this->createCreditCardPayment('visa', '4444333322221111', '123', new \DateTime(((int)$now->format('Y')+1) . '-05'));
    $payment->execute();
  }
  public function testValidVisa3DS() {
    $now = \date_create();
    $payment = $this->createCreditCardPayment('visa', '4444333322221111', '123', new \DateTime(((int)$now->format('Y')+1) . '-06'));
    $payment->finish_callback = 'var_export';
    $payment->execute();
  }
}
