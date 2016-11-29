<?php

namespace Drupal\mpay24_payment;

class SuccessRequest extends BaseRequest {
  public function validate(array $parameters) {
    $this->log();
  }

  public function mapppedStatus() {
    return \PAYMENT_STATUS_SUCCESS;
  }
}
