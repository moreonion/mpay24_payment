<?php

namespace Drupal\mpay24_payment;

abstract class BaseRequest {
  public $parameters;
  protected $payment;
  public function __construct(\Payment $payment) {
    $this->parameters = array();
    $this->payment = $payment;
  }

  public function log() {
    $parameters = array();
    foreach ($this->parameters as $key => $value) {
      $parameters[] = "$key => '$value'";
    }
    $parameters = implode(', ', $parameters);
    \watchdog(
      'mpay24_payment',
      '!class: Payment(!pid) -> !parameters',
      array('!class' => get_called_class(), '!parameters' => $parameters, '!pid' => $this->payment->pid),
      WATCHDOG_INFO
    );
  }

  public function setStatus() {
    $payment = $this->payment;
    if (isset($this->parameters['MPAYTID'])) {
      $payment->mpaytid = $this->parameters['MPAYTID'];
    }
    $status = $this->mapppedStatus();
    if ($payment->getStatus()->status != $status) {
      $payment->setStatus(new \PaymentStatusItem($status, REQUEST_TIME, $payment->pid));
    }
  }
}
