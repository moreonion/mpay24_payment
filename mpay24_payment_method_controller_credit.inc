
<?php

module_load_include('inc', 'mpay24_payment', 'mpay24_payment_method_controller_common');

/**
 * @file
 * MPay24 payment method controller for credit card payments
 */
class PaymentMethodControllerMPay24CreditCard extends PaymentMethodControllerMPay24 {
  public $payment_configuration_form_elements_callback = 'mpay24_payment_payment_configuration_form_elements';

  function __construct() {
    parent::__construct();

    $this->title = t('mPay24');
  }


  /**
   * Implements PaymentMethodController::validate().
   */
  function validate(Payment $payment, PaymentMethod $payment_method, $strict) {
  }


  /**
   * Implements PaymentMethodController::execute().
   */
  function execute(Payment $payment) {
    $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_REDIRECT));
    dpm($payment);
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

  foreach($values as $key => $value) {
    if(empty($value)) {
      form_error($element[$key], t('@name is required', array('@name' => $element[$key]['#title'])));
    }
    $form_state['payment']->method_data[$key] = $value;
  }

}
