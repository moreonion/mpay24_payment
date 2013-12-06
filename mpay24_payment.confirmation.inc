<?php

/**
 * @file
 * Implements mPay24s confirmation interface, see section 4.2.6 of the spec.
 *
 * @todo: Read Appendix A: Confirmation Interface in the spec for security
 *   recommendations
 */

use Drupal\mpay24_payment\ConfirmationRequest;

function mpay24_payment_confirmation_interface_handler($payment) {
  drupal_add_http_header('Content-Type', 'text/plain');
  try {
    $request = new ConfirmationRequest($payment);
    $request->validate($_GET);
    $request->setStatus();
    print 'OK: ';
  } catch (PaymentException $e) {
    watchdog('mpay24_payment', 'Exception in confirmation request: ' . $e->getMessage());
    print 'ERROR: ' . $e->getMessage();
  }
  exit(0); // need to get unthemed output.
}

function mpay24_payment_confirmation_wait(Payment $payment) {
  if (!payment_status_is_or_has_ancestor($payment->getStatus()->status,
      PAYMENT_MPAY24_STATUS_REDIRECT)) {
    throw new PaymentException(
      t("Confirmation polling for non-redirected payment @pid requested",
        array('@pid' => $payment->pid)));
  };

  if (isset($payment->context_data['context'])) {
    $success_url = $payment->context_data['context']->getSuccessUrl();
  } else {
    //@TODO: use some default success-url.
    throw new Exception(
      t("No redirection url set for redirected payment @pid.",
        array('@pid' => $payment->pid)));
  }

  $url_options = array(
    'absolute' => true,
    'https'    => true,
  );
  drupal_add_js(
    array(
      'mpay24_payment' => array(
        'pid' => $payment->pid,
        'timeout' => empty($payment->method->controller_data['timeout']) ? 30 : $payment->method->controller_data['request_timeout'],
        'success_url' => $success_url,
        'error_url'   => url(MPAY24_PAYMENT_ERROR_URL, $url_options),
        'status_url' => url(MPAY24_PAYMENT_CONFIRMATION_STATUS_URL, $url_options),
      ),
    ), 'setting');

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
  $timeout = $payment->method->controller_data['request_timeout'];

  $payment->method->controller->pollStatus($payment);
  if (($status->created + $timeout) < time()) {
    $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_EXPIRED, time(), $payment->pid));
    entity_save('payment', $payment);
  }
  $status = $payment->getStatus();

  switch ($status->status) {
    case PAYMENT_STATUS_FAILED:
      drupal_json_output(array('status' => 'error'));
      break;
    case PAYMENT_STATUS_EXPIRED:
      drupal_json_output(array('status' => 'timeout'));
      break;
    case PAYMENT_STATUS_SUCCESS:
      drupal_json_output(array('status' => 'success'));
      break;
    default:
      drupal_json_output(array('status' => NULL));
  }
  exit(0);
}

function mpay24_payment_confirmation_error(Payment $payment) {
  try {
    $request = new ConfirmationRequest($payment);
    $request->validate($_GET);
    $request->setStatus();
  } catch(PaymentException $e) {
    watchdog('mpay24_payment', 'Exception in error request: ' . $e->getMessage());
    if ($payment->getStatus() != PAYMENT_STATUS_FAILED) {
      $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_FAILED, REQUEST_TIME, $payment->pid));
    }
  }

  watchdog('mpay24_payment', 'Reported an error to the user of Payment @pid.', array('@pid' => $payment->pid));

  if (isset($payment->context_data['context'])) {
    $context = $payment->context_data['context'];
    $link = $context->reenterLink($payment);
    $message = t('An error occured while processing your payment. Please !tryagainlink or contact the site administrator if the problem persists.', array(
      '!tryagainlink' => l('try again', $link['path'], $link),
    ));
  } else {
    $message = t('An error occured while processing your payment. Contact the site administrator if the problem persists.');
  }
  $output = array(
    '#markup' => $message,
  );
  return $output;
}
