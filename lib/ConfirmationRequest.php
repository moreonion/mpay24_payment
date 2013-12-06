<?php

namespace Drupal\mpay24_payment;

class ConfirmationRequest {
  public function __construct(\Payment $payment) {
    $this->payment = $payment;
    $this->parameters = array();
  }
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

    // check if TID matches.
    if ($this->payment->tid != $this->parameters['TID']) {
      $message = t('Contfirmation Interface: Mismatching TID for payment ID !pid.', array(
        '!pid' => $this->payment->pid,
      ));
      throw new \PaymentException($message);
    }
    $this->log();
  }

  public function log() {
    $parameters = array();
    foreach ($this->parameters as $key => $value) {
      $parameters[] = "$key => '$value'";
    }
    $parameters = implode(', ', $parameters);
    \watchdog(
      'mpay24_payment',
      'Confirmation Interface: Payment(!pid) -> !parameters',
      array('!parameters' => $parameters, '!pid' => $this->payment->pid),
      WATCHDOG_INFO
    );
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

  public function setStatus() {
    $payment = $this->payment;
    if (isset($this->parameters['MPAYTID'])) {
      $payment->mpaytid = $this->parameters['MPAYTID'];
    }
    $status = $this->mapppedStatus();
    if ($payment->getStatus()->status != $status) {
      $payment->setStatus(new \PaymentStatusItem($status, REQUEST_TIME, $payment->pid));
      \entity_save('payment', $payment);
    }
  }
}
