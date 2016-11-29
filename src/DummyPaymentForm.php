<?php

namespace Drupal\mpay24_payment;


class DummyPaymentForm implements \Drupal\payment_forms\PaymentFormInterface {

  /**
   * Add form elements to the $element Form-API array.
   */
  public function form(array $element, array &$form_state, \Payment $payment) {
    $element['validate_timeout'] = array(
      '#type' => 'select',
      '#title' => 'Validate timeout.',
      '#description' => 'Timeout before redirecting to another URL.',
      '#options' => array(0 => 'Zero', 1 => '1 second', 5 => '5 seconds', 30 => '30 seconds', 120 => '2 minutes'),
    );

    $element['action'] = array(
      '#type' => 'select',
      '#title' => 'Submit action',
      '#description' => 'What should happen after the form validation?',
      '#options' => array(
        'success_page' => 'Redirect to the thank you page',
        'error_page' => 'Redirect to an error page',
        'poll' => 'Redirect to the polling interface',
        'error' => 'Return something erronous',
      ),
    );

    $element['poll'] = array(
      '#type' => 'fieldset',
      '#title' => 'Poll behavior',
      '#description' => 'Beavior during polling',
      '#states' => array(
        'visible' => array(
          'select[id$=-paymentmethodcontrollermpay24dummy-action]' => array('value' => 'poll'),
        ),
      ),
    );

    $element['poll']['timeout'] = array(
      '#type' => 'select',
      '#title' => 'Timeout',
      '#description' => 'Timeout before setting the target status.',
      '#options' => array(0 => 'Zero', 1 => '1 second', 5 => '5 seconds', 30 => '30 seconds', 120 => '2 minutes'),
    );
    $options = array();
    foreach (payment_statuses_info() as $info) {
      $options[$info->status] = $info->title;
    }
    $element['poll']['status'] = array(
      '#type' => 'select',
      '#title' => 'Target status',
      '#description' => 'Status to be set after the timeout is up.',
      '#options' => $options,
    );
    return $element;
  }

  /**
   * Validate the submitted values.
   */
  public function validate(array $element, array &$form_state, \Payment $payment) {
    $values =& $form_state['values'];
    foreach ($element['#parents'] as $key) {
      $values =& $values[$key];
    }
    $payment->context_data['method_data'] = $values;
  }

}
