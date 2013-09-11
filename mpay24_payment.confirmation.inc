<?php


/**
 * @file
 * Implements mPay24s confirmation interface, see section 4.2.6 of the spec.
 *
 * @todo: Read Appendix A: Confirmation Interface in the spec for security
 *   recommendations
 */

function mpay24_payment_confirmation_interface_handler($payment) {
  try {
    if (!isset($_GET['TID']) || $payment->tid != $_GET['TID']) {
      throw new PaymentException(
        'Can\'t handle confirmation request without a valid transaction ID.');
    }

    $request = array();
    $transaction_parameters = array(
      'OPERATION', 'STATUS', 'TID', 'PRICE', 'CURRENCY', 'MPAYTID');
    foreach ($transaction_parameters as $transaction_parameter) {
      if (isset($_GET[$transaction_parameter]) == TRUE) {
        $request[$transaction_parameter] = check_plain($_GET[$transaction_parameter]);
      } else {
        throw new PaymentException(
          t(' Confirmation Interface: Mandatory parameter @arg_name missing.',
            array('@arg_name' => $transaction_parameter)));
      }
    }

    foreach (array('LANGUAGE', 'MSG') as $transaction_parameter) {
      if (isset($_GET[$transaction_parameter]) == TRUE) {
        $request[$transaction_parameter] = check_plain($_GET[$transaction_parameter]);
      }
      else { $request[$transaction_parameter] = NULL; }
    }

    $payments = entity_load('payment', array($pid));
    $payment = &$payments[$pid];
    
    $new_status = PAYMENT_STATUS_UNKNOWN;
    switch ($request['STATUS']) {
      case 'ERROR':
        $status = PAYMENT_STATUS_FAILED;
        break;
      case 'SUSPENDED':
      case 'TIMEOUT':
        $status = PAYMENT_STATUS_EXPIRED;
        break;
      case 'BILLED':
      case 'CREDITED':
        $status = PAYMENT_STATUS_SUCCESS;
        break;
      default:
    }
    if ($request['MPAYTID']) {
      $payment->mpaytid = (int) $request['MPAYTID'];
    }
    $payment->setStatus(new PaymentStatusItem($status, REQUEST_TIME, $payment->pid));

    watchdog(
      'mpay24_payment',
      "Confirmation Interface: OPERATION: \"@o\", STATUS: \"@s\", TID: \"@t\", PRICE: \"@p\", CURRENCY: \"@c\", MPAYTID: \"@m\"",
      array(
        '@o' => $request['OPERATION'],
        '@s' => $request['STATUS'],
        '@t' => $request['TID'],
        '@p' => $request['PRICE'],
        '@c' => $request['CURRENCY'],
        '@m' => $request['MPAYTID'],
      ),
      WATCHDOG_INFO
    );

    drupal_add_http_header('Content-Type', 'text/plain');
    print 'OK: ';
    exit(0); // need to get unthemed output.
  } catch (PaymentException $e) {
    watchdog('mpay24_payment', 'Exception: ' . $e->getMessage());
    drupal_add_http_header('Content-Type', 'text/plain');
    print 'ERROR: ' . $e->getMessage();
    exit(0); // need to get unthemed output.
  }
}

function mpay24_payment_confirmation_wait(Payment $payment) {
  if (!payment_status_is_or_has_ancestor($payment->getStatus()->status,
      PAYMENT_MPAY24_STATUS_REDIRECT)) {
    throw new PaymentException(
      t("Confirmation polling for non-redirected payment @pid requested",
        array('@pid' => $payment->pid)));
  };

  // @todo: Get the url from somewhere else, to make the method independent from
  // the payment context.
  if(!empty($payment->context_data['webform']['redirect_url'])) {
    $success_url = url($payment->context_data['webform']['redirect_url'],
                     array('absolute' => TRUE));
  } else {
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
  watchdog('mpay24_payment', 'Reported an error to the user of Payment @pid.', array('@pid' => $payment->pid));

  return array(
    '#markup' => '<div class="mo-dialog-wrapper visible"><div class="mo-dialog-content loading"><p>'.t('An error occured. Please try again later.').'</p></div></div>'
  );
}
