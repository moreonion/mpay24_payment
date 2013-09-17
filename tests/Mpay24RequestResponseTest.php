<?php

class Mpay24RequestResponseTest extends \DrupalUnitTestCase {

  public function setUp() {
    parent::setUp();

    drupal_static_reset('form_set_error');
  }
  // helper methods

  function getOkResponse() {
    $response = new StdClass();
    $response->code = '200';
    $response->data = "STATUS=ERROR&RETURNCODE=BRAND%5FNOT%5FCORRECT&ERRNO=&EXTERNALSTATUS=&MPAYTID=";
    $response->data = "STATUS=OK&RETURNCODE=OK&ERRNO=&EXTERNALSTATUS=&MPAYTID=";
    return $response;
  }
  function paymentInstance() {
    /* $line_item = new PaymentLineItem(array( */
    /*   'name'         => 'donation', */
    /*   'description'  => 'donation', */
    /*   'amount'       => 0, */
    /*   'quantity'     => 1, */
    /*   'tax_rate'     => 0, */
    /* )); */
    /* $payment = new Payment(array( */
    /*   'currency_code  ' => 'EUR', */
    /*   'description'     => 'donation', */
    /*   'finish_callback' => 'webform_component_paymethod_select_payment_finish', */
    /* )); */
    /* $payment->setLineItem($line_item); */
    /* ---- DOES THE SAME AS: ---- */
    $payment = webform_component_paymethod_select_create_payment();

    $payment->line_items['donation']->amount = 5;

    $methods = entity_load('payment_method', FALSE, array('controller_class_name' => 'PaymentMethodControllerMPay24CreditCard'));
    $method = array_shift($methods);
    $payment->method = $method;

    $stub_request = $this->getMock('\mpay24\Request', array('send'));
    $stub_request->payment = $payment;
    $stub_request->expects($this->once())
      ->method('send')
      ->will($this->returnCallback(function() {
        $response = new StdClass();
        $response->code = '200';
        $response->data = "STATUS=ERROR&RETURNCODE=BRAND%5FNOT%5FCORRECT&ERRNO=&EXTERNALSTATUS=&MPAYTID=";
        $response->data = "STATUS=OK&RETURNCODE=REDIRECT&LOCATION=https%3A%2F%2Flocalhost";
        return $response;
      }
        ));
    $payment->test_request = $stub_request;

    $stub_controller = $this->getMock('PaymentMethodControllerMPay24CreditCard', array('getResponse'));
    $stub_controller->expects($this->any())
      ->method('getResponse')
      ->will($this->returnCallback(function($payment) {
        /* $obj = $stub_request; */
  return new \mpay24\Response($payment, \mpay24\Request::creditCard($payment));
      }
        ));
    $payment->method->controller = $stub_controller;


    $payment->method_data['first_name'] = 'Vorname';
    $payment->method_data['last_name'] = 'Nachname';
    $payment->context_data['webform'] = array(
      'redirect_url' => '',
    );

    return $payment;
  }

  function validElement() {
    $element = array(
      '#parents' => array('submitted', 'PaymentMethodControllerMPay24CreditCard'),
      'issuer' => array(
        '#parents' => array('submitted', 'PaymentMethodControllerMPay24CreditCard', 'issuer'),
        '#options' => array(
          '1' => 'Visa',
          '2' => 'MasterCard',
          '3' => 'American Express',
        ),
      ),
      'credit_card_number' => array(
        '#parents' => array('submitted', 'PaymentMethodControllerMPay24CreditCard', 'credit_card_number'),
      ),
      'secure_code' => array(
        '#parents' => array('submitted', 'PaymentMethodControllerMPay24CreditCard', 'secure_code'),
      ),
      'expiry_date' => array(
        '#parents' => array('submitted', 'PaymentMethodControllerMPay24CreditCard', 'expiry_date'),
      ),
    );

    return $element;
  }

  function validFormState() {
    $payment = $this->paymentInstance();
    $now = date_create();
    $form_state = array(
      'values' => array(
        'submitted' => array(
          'PaymentMethodControllerMPay24CreditCard' => array(
            'issuer' => '1',
            'credit_card_number' => '4444333322221111',
            'secure_code' => '123',
            'expiry_date' => '05/'.((int)$now->format('y')+1),
          ),
        ),
      ),
      'payment' => &$payment,
    );

    return $form_state;
  }

  // ---------- tests ------------


  function testRequest() {
    // get a valid form_state

    # TODO call it, when the test is coded
    #$payment = $this->paymentInstance();

    #var_dump($payment->method->controller->validate($payment, $payment->method, true));


  }
}
