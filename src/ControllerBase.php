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
   * Helper for entity_load().
   */
  public static function load($entities) {
    $pmids = array();
    foreach ($entities as $method) {
      if ($method->controller instanceof PaymentMethodControllerMPay24) {
        $pmids[] = $method->pmid;
      }
    }
    if ($pmids) {
      $query = db_select('mpay24_payment_payment_method_controller', 'controller')
        ->fields('controller')
        ->condition('pmid', $pmids);
      $result = $query->execute();
      while ($data = $result->fetchAssoc()) {
        $method = $entities[$data['pmid']];
        unset($data['pmid']);
        if ($data['testmode']) {
          $data['merchantid'] = '9' . substr($data['merchantid'], 1);
        }
        $method->controller_data = (array) $data;
        $method->controller_data += $method->controller->controller_data_defaults;
      }
    }
  }

  /**
   * Helper for entity_insert().
   */
  public function insert($method) {
    $method->controller_data += $this->controller_data_defaults;

    $query = db_insert('mpay24_payment_payment_method_controller');
    $values = array_merge($method->controller_data, array('pmid' => $method->pmid));
    $query->fields($values);
    $query->execute();
  }

  /**
   * Helper for entity_update().
   */
  public function update($method) {
    $query = db_update('mpay24_payment_payment_method_controller');
    $values = array_merge($method->controller_data, array('pmid' => $method->pmid));
    $query->fields($values);
    $query->condition('pmid', $method->pmid);
    $query->execute();
  }

  /**
   * Helper for entity_delete().
   */
  public function delete($method) {
    db_delete('mpay24_payment_payment_method_controller')
      ->condition('pmid', $method->pmid)
      ->execute();
  }

  /**
   * Allows the controller to act when the payment status is polled.
   */
  public function pollStatus(Payment $payment) {
  }
}
