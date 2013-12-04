<?php

namespace Drupal\mpay24_payment;

class CreditCardPaymentTest extends \DrupalUnitTestCase {
  public function testValidVisa() {
    $controller = new \PaymentMethodControllerMPay24CreditCard();
    $method = new \PaymentMethod();
    $method->controller = $controller;
    $method->controller_data = array(
      'testmode' => TRUE,
      'merchantid' => '91485' 
    );
    $payment = new \Payment();
    $payment->method = $method;
    $payment->setLineItem(new \PaymentLineItem(array(
      'amount' => 5,
      'description' => 'Test payment',
      'name' => 'test',
    )));
    $now = \date_create();
    $payment->method_data += array(
      'issuer' => 'visa',
      'credit_card_number' => '4444333322221111',
      'secure_code' => '123',
      'expiry_date' => '05/'.((int)$now->format('y')+1),
    );
    $payment->form_state = array();
    $payment->execute();
  }
}
