<?php

/**
 * @file
 */

namespace Drupal\mpay24_payment;

/**
 *
 */
class CreditCardForm extends \Drupal\payment_forms\CreditCardForm {
  static protected $issuers = array(
    'visa' => 'Visa',
    'mastercard' => 'MasterCard',
  );
  public function validate(array $element, array &$form_state, \Payment $payment) {
    parent::validate($element, $form_state, $payment);

    if (form_get_errors()) {
      return;
    } else {
      $this->validate_online_via_mpay24($element, $form_state, $payment);
    }
  }

  protected function validate_online_via_mpay24(array $element, array &$form_state, \Payment $payment) {
    try {
      $payment->method->validate($payment, TRUE);
    } catch (\mpay24\PaymentMpay24ApiException $e) {
      $code = $e->getErrorCode();
      switch ($code) {
        case 'IDENTIFIER_NOT_CORRECT':
          form_error($element['credit_card_number'], t('Please enter a valid credit card number.'));
          break;
        case 'CVC_NOT_CORRECT':
          form_error($element['secure_code'], t('Please enter the correct secure code.'));
          break;
      }
    }
  }
}
