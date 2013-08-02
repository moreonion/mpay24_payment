<?php


/**
 * @file
 * MPay24 payment method controller.
 * This controller executes a payment trough mpay24s full integration API.
 */
class PaymentMethodControllerMPay24 extends PaymentMethodController {
  public $controller_data_defaults = array(
    'merchantid'             => '',
    'testmode'               => 1,
    'request_timeout'        => 30.0,
  );

  public $payment_method_configuration_form_elements_callback = 'mpay24_payment_payment_method_configuration_form_elements';


  function __construct() {
    $this->title = t('mPay24');
    $this->description = t("A Payment API method to transfer money using the payment provider https://mpay24.com");

    $this->currencies = array_fill_keys(array('EUR'), array());
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
      if (($method->controller->name === 'PaymentMethodControllerMPay24CreditCard')) {
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
        $method->controller_data = (array) $data;
        unset($method->controller_data['pmid']);
      }
    }
  }

  /**
   * Helper for entity_insert().
   */
  public static function insert($method) {
    if (($method->controller->name === 'PaymentMethodControllerMPay24CreditCard')) {
      $method->controller_data += $method->controller->controller_data_defaults;

      $query = db_insert('mpay24_payment_payment_method_controller');
      $values = array_merge($method->controller_data, array(
                  'pmid' => $method->pmid,
                ));
      $query->fields($values);
      $query->execute();
    }
  }

  /**
   * Helper for entity_update().
   */
  public static function update($method) {
    if (($method->controller->name === 'PaymentMethodControllerMPay24CreditCard')) {
      $query = db_update('mpay24_payment_payment_method_controller');
      $values = array_merge($method->controller_data, array(
                  'pmid' => $method->pmid,
                ));
      $query->fields($values);
      $query->condition('pmid', $method->pmid);
      $query->execute();
    }
  }

  /**
   * Helper for entity_delete().
   */
  public static function delete($entity) {
    if (($method->controller->name === 'PaymentMethodControllerMPay24CreditCard')) {
      db_delete('mpay24_payment_payment_method_controller')
        ->condition('pmid', $entity->pmid)
        ->execute();
    }
  }
}