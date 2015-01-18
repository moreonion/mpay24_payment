<?php
require_once dirname(__FILE__) . '/mpay24.api.inc';

class PaymentMethodControllerMPay24Dummy extends  PaymentMethodControllerMPay24{
  public $payment_method_configuration_form_elements_callback = 'mpay24_payment_dummy_configuration_form_elements';
  public $payment_configuration_form_elements_callback = 'mpay24_payment_dummy_form_elements';
  public function __construct() {
    $this->title = 'Mpay24 Dummy';
    $this->description = 'This payment method allows to mock server-side behavior.';
  }
  
  /**
   * Implements PaymentMethodController::validate().
   */
  function validate(Payment $payment, PaymentMethod $payment_method, $strict) {
    if (!$strict)
      return;

    sleep($payment->context_data['method_data']['validate_timeout']);
  }
  
  /**
   * Implements PaymentMethodController::execute().
   */
  function execute(Payment $payment) {
    $request = new \mpay24\Request($payment);

    $data = &$payment->context_data['method_data'];
    $options['query']['TID'] = $payment->tid;
    $redirect = NULL;
    switch ($data['action']) {
      case 'error':
        echo 'Some test output';
        exit;
      case 'poll':
        $payment->setStatus(new \PaymentStatusItem(PAYMENT_MPAY24_STATUS_REDIRECT));
        $path = MPAY24_PAYMENT_CONFIRMATION_WAIT_URL . (int) $payment->pid;
        $redirect = array($path, $options);
        break;
      case 'error_page':
        $redirect = array($request->getErrorURL(), $options);
        break;
      case 'success_page':
      default:
        $redirect = array($request->getSuccessURL(), $options);;
    }
    if ($redirect) {
      $payment->contextObj->redirect($redirect[0], $redirect[1]);
    }
  }
  
  /**
   * Set the target status once the timeout has run out.
   */
  public function pollStatus(Payment $payment) {
    $status = $payment->getStatus();
    $data = &$payment->context_data['method_data']['poll'];
    if ($status->created + $data['timeout'] < REQUEST_TIME && $status->status != $data['status']) {
      $payment->setStatus(new PaymentStatusItem($data['status'], time(), $payment->pid));
      entity_save('payment', $payment);
    }
  }
}

function mpay24_payment_dummy_form_elements(array $element, array &$form_state) {
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

function mpay24_payment_dummy_form_elements_validate(array $element, array &$form_state) {
  $values =& $form_state['values'];
  foreach ($element['#parents'] as $key) {
    $values =& $values[$key];
  }
  $form_state['payment']->context_data['method_data'] = $values;
  $form_state['payment']->form_state = &$form_state;
}

/**
 * Form build callback: implements
 * PaymentMethodController::payment_method_configuration_form_elements_callback.
 *
 * @return array
 *   A Drupal form.
 */
function mpay24_payment_dummy_configuration_form_elements(array $form, array &$form_state) {
  $controller_data = $form_state['payment_method']->controller_data;
  $elements['request_timeout'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Timeout for payment requests'),
    '#description'   => t('Timeout for the payment request to MPay24 (in Seconds).'),
    '#default_value' => isset($controller_data['request_timeout']) ? $controller_data['request_timeout'] : 100.0,
  );

  return $elements;
}

/**
 * Implements form validate callback for
 * payment_mpay24_payment_method_configuration_form_elements().
 */
function mpay24_payment_dummy_configuration_form_elements_validate(array $element, array &$form_state) {
  $values = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  $form_state['payment_method']->controller_data['request_timeout'] = $values['request_timeout'];
}
