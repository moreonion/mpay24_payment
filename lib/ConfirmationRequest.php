<?php

namespace Drupal\mpay24_payment;

class ConfirmationRequest extends BaseRequest {
  public function validate(array $parameters) {
    // mandatory parameters
    foreach (array('OPERATION', 'STATUS', 'TID', 'PRICE', 'CURRENCY', 'MPAYTID') as $key) {
      if (isset($parameters[$key])) {
        $this->parameters[$key] = $parameters[$key];
      } else {
        throw new \PaymentException(
          t(' Confirmation Interface: Mandatory parameter @arg_name missing.',
          array('@arg_name' => $key))
        );
      }
    }

    // optional parameters
    foreach (array('LANGUAGE', 'MSG') as $key) {
      $this->parameters[$key] = isset($parameters[$key]) ? $parameters[$key] : NULL;
    }

    $this->log();
  }

  public function mapppedStatus() {
    switch ($this->parameters['STATUS']) {
      case 'ERROR':
        $status = \PAYMENT_STATUS_FAILED;
        break;
      case 'SUSPENDED':
      case 'TIMEOUT':
        $status = \PAYMENT_STATUS_EXPIRED;
        break;
      case 'BILLED':
      case 'RESERVED':
      case 'CREDITED':
        $status = \PAYMENT_STATUS_SUCCESS;
        break;
      default:
        $status = \PAYMENT_STATUS_UNKNOWN;
    }
    return $status;
  }


}
