<?php

namespace Drupal\mpay24_payment;

class ControllerConfigurationForm implements \Drupal\payment_forms\MethodFormInterface {

  public function form(array $element, array &$form_state, \PaymentMethod $method) {
    $controller_data = $method->controller_data;

    $element['merchantid'] = array(
      '#type' => 'textfield',
      '#title' => t('MPay24 MerchantID'),
      '#description' => t('Available from MPay24 support.'),
      '#default_value' => isset($controller_data['merchantid']) ? $controller_data['merchantid'] : '',
    );

    $element['testmode'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use the test API') . ' (test.mpay24.com)',
      '#description' => t('Requires a different MerchantID.'),
      '#default_value' => isset($controller_data['testmode']) ? $controller_data['testmode'] : '',
    );

    $element['request_timeout'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Timeout for payment requests'),
      '#description'   => t('Timeout for the payment request to MPay24 (in Seconds).'),
      '#default_value' => isset($controller_data['request_timeout']) ? $controller_data['request_timeout'] : 100.0,
    );

    return $element;
  }

  public function validate(array $element, array &$form_state, \PaymentMethod $method) {
    $values = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
    $method->controller_data['request_timeout'] = $values['request_timeout'];
    $method->controller_data['merchantid'] = $values['merchantid'];
    $method->controller_data['testmode'] = $values['testmode'];
  }

}
