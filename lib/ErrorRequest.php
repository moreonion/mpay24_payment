<?php

namespace Drupal\mpay24_payment;

class ErrorRequest extends BaseRequest {
  public function validate(array $parameters) {
    // optional parameters
    foreach (array('ERRTEXT', 'EXTERNALSTATUS', 'LANGUAGE') as $key) {
      $this->parameters[$key] = isset($parameters[$key]) ? $parameters[$key] : NULL;
    }

    $this->log();
  }

  public function mapppedStatus() {
    return \PAYMENT_STATUS_FAILED;
  }
}
