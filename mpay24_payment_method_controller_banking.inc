<?php

module_load_include('inc', 'mpay24_payment', 'mpay24.api');

/**
 * @file
 * MPay24 payment method controller for online banking payments.
 */
class PaymentMethodControllerMPay24OnlineBanking extends PaymentMethodControllerMPay24 {
  public $payment_configuration_form_elements_callback = 'mpay24_payment_banking_configuration_form_elements';

  function __construct() {
    parent::__construct();

    $this->title = t('mPay24 Online Banking');
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
    $payment->form_state['redirect'] = $response->uri;
  }
}

/**
 * Implements
 * PaymentMethodController::payment_configuration_form_elements_callback.
 */
function mpay24_payment_banking_configuration_form_elements(array $element, array &$form_state) {
  $element = array(
    '#type'             => 'fieldset',
    '#collapsible'	=> TRUE,
    '#collapsed'	=> TRUE,
    '#title'		=> t('Online Banking'),
  );

  $element['redirection_info'] = array(
    '#type'	=> 'markup',
    '#markup'  => t('Nach einem Klick auf "Fertig" werden Sie zu unserem externen'
                . ' Anbieter weitergleitet um die Zahlung abzuschließen.'),
    '#weight'	=> 0,
  );
  return $element;
}

/**
 * Implements form validate callback for
 * mpay24_payment_banking_configuration_form_elements
 */
function mpay24_payment_banking_configuration_form_elements_validate(array $element, array &$form_state) {
  // safe reference to form_state, needed for later execute()
  $form_state['payment']->form_state = &$form_state;
}

