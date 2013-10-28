<?php
/**
 * @file
 *
 * @author    Paul Haerle <phaer@phaer.org>
 * @copyright Copyright (c) 2013 copyright
 */

namespace Drupal\payment_forms;

/**
 *
 */
class MPay24CreditCardForm extends CreditCardForm {
  public function validateForm(array &$element, array &$form_state) {
    parent::validateForm($element, $form_state);

    if (form_get_errors()) {
      return;
    } else {
      $this->validate_online_via_mpay24($element, $form_state);
    }

    // needed for later execute()
    $form_state['payment']->form_state = &$form_state;
  }

  protected function validate_online_via_mpay24(array $element,array &$form_state) {
    try {
      $form_state['payment']->method->validate($form_state['payment'], TRUE);
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
