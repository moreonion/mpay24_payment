<?php

/**
 * @file
 * MPay24 payment method controller.
 * This controller executes a payment trough mpay24s full integration API.
 */

namespace Drupal\mpay24_payment;

class ControllerBase extends \PaymentMethodController {

  public $controller_data_defaults = array(
    'merchantid'             => '',
    'testmode'               => 1,
    'request_timeout'        => 30.0,
  );

  public $payment_method_configuration_form_elements_callback = 'payment_forms_method_configuration_form';
  public $payment_configuration_form_elements_callback = 'payment_forms_payment_form';



  function __construct() {
    require_once drupal_get_path('module', 'mpay24_payment') . '/mpay24.api.inc';

    $this->title = t('mPay24');
    $this->description = t("A Payment API method to transfer money using the payment provider https://mpay24.com");

    $this->currencies = array_fill_keys(array('EUR'), array());
  }

  public function configurationForm() {
    return new ControllerConfigurationForm();
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
  }

  /**
   * Allows the controller to act when the payment status is polled.
   */
  public function pollStatus(Payment $payment) {
  }

}
