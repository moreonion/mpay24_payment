<?php

/**
 * @file
 * Implements mPay24s confirmation interface, see section 4.2.6 of the spec.
 *
 * @todo: Read Appendix A: Confirmation Interface in the spec for security
 *   recommendations
 */

use Drupal\mpay24_payment\ConfirmationRequest;
use Drupal\mpay24_payment\ErrorRequest;
use Drupal\mpay24_payment\SuccessRequest;

function mpay24_payment_confirmation_interface_handler(Payment $payment) {
  drupal_add_http_header('Content-Type', 'text/plain');
  try {
    $request = new ConfirmationRequest($payment);
    $request->validate($_GET);
    $request->setStatus();
    entity_save('payment', $payment);
    print 'OK: ';
  } catch (PaymentException $e) {
    watchdog('mpay24_payment', 'Exception in confirmation request: ' . $e->getMessage());
    print 'ERROR: ' . $e->getMessage();
  }
  exit(0); // need to get unthemed output.
}

function mpay24_payment_confirmation_wait(Payment $payment) {
  $options['query']['TID'] = $_GET['TID'];
  $status_url = url(MPAY24_PAYMENT_CONFIRMATION_STATUS_URL . $payment->pid, $options);
  drupal_add_js(array('mpay24_payment' => array('status_url' => $status_url)), 'setting');

  return array(
    '#markup' => '<div class="mo-dialog-wrapper visible"><div class="mo-dialog-content loading"><p>'.t('Please wait while we are proccessing your payment.').'</p></div></div>',
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'mpay24_payment') . '/js/confirmation.js'
      ),
    ),
  );
}

function mpay24_payment_confirmation_status(Payment $payment) {
  $status = $payment->getStatus();
  $payment->method->controller->pollStatus($payment);

  $output = array();
  $args['query']['TID'] = $payment->tid;
  switch ($status->status) {
    case PAYMENT_STATUS_FAILED:
    case PAYMENT_STATUS_EXPIRED:
      $args['query']['ERRTEXT'] = $status->status;
      $output['status'] = 'redirect';
      $output['url'] = url(MPAY24_PAYMENT_ERROR_URL . $payment->pid, $args);
      break;
    case PAYMENT_STATUS_SUCCESS:
      $output['status'] = 'redirect';
      $output['url'] = url(MPAY24_PAYMENT_SUCCESS_URL . $payment->pid, $args);
      break;
    default:
      $output['status'] = 'poll';
  }

  $timeout = $payment->method->controller_data['request_timeout'];
  if ($output['status'] == 'poll' && ($status->created + $timeout) < time()) {
    $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_EXPIRED, time(), $payment->pid));
    entity_save('payment', $payment);
  }

  drupal_json_output($output);
  exit(0);
}

function mpay24_payment_confirmation_error(Payment $payment) {
  $request = new ErrorRequest($payment);
  $request->validate($_GET);
  watchdog('mpay24_payment', 'Reported an error to the user of Payment @pid.', array('@pid' => $payment->pid));
  $request->setStatus();
  $payment->finish();
}

function mpay24_payment_confirmation_success(Payment $payment) {
  $request = new SuccessRequest($payment);
  $request->validate($_GET);
  $request->setStatus();
  $payment->finish();
}
