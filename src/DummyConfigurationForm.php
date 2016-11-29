<?php

namespace Drupal\mpay24_payment;

class DummyConfigurationForm implements \Drupal\payment_forms\MethodFormInterface {

  /**
   * Add form elements to the $element Form-API array.
   */
  public function form(array $element, array &$form_state, \PaymentMethod $method) {
    $controller_data = $method->controller_data;
    $element['request_timeout'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Timeout for payment requests'),
      '#description'   => t('Timeout for the payment request to MPay24 (in Seconds).'),
      '#default_value' => isset($controller_data['request_timeout']) ? $controller_data['request_timeout'] : 100.0,
    );
    return $element;
  }

  /**
   * Validate the submitted values.
   */
  public function validate(array $element, array &$form_state, \PaymentMethod $method) {
    $values = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
    $method->controller_data['request_timeout'] = $values['request_timeout'];
  }

}
