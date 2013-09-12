<?php

class Mpay24ValidationsTest extends \DrupalUnitTestCase {

  // helper methods

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
    $payment->method = array_shift($methods);
    $pmid = $payment->method->pmid;
    //$method = entity_load_single('payment_method', 2);

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

  function testValidCreditCard() {
    // get a valid form_state
    $element = $this->validElement();
    $form_state = $this->validFormState();

    mpay24_payment_payment_configuration_form_elements_validate($element, $form_state);

    $form = &drupal_static('form_set_error', array());

    // assertion: no form_errors
    $this->assertEmpty($form);
  }

  function testInvalidIssuer() {
    // get a valid form_state
    $element = $this->validElement();
    $form_state = $this->validFormState();

    // invalidate issuer
    $form_state['values']['submitted']['PaymentMethodControllerMPay24CreditCard']['issuer'] = '2';

    mpay24_payment_payment_configuration_form_elements_validate($element, $form_state);

    $form = &drupal_static('form_set_error', array());

    // assertion: form_errors in credit_card_number
    $this->assertGreaterThan(0, count($form));
    $this->assertEquals(implode('][', $element['credit_card_number']['#parents']), array_shift(array_keys($form)));
  }
}
