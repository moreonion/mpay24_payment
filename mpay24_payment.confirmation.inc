<?php


/**
 * @file
 * Implements mPay24s confirmation interface, see section 4.2.6 of the spec.
 *
 * @todo: Read Appendix A: Confirmation Interface in the spec for security
 *   recommendations
 * @todo: Use SSL for the Confirmation Interface.
 * @todo: Use unique token or user/pass for the Confirmation Interface.
 */

function mpay24_payment_confirmation_interface_handler() {
  try {
    if (isset($_GET['TID']) == FALSE) {
      throw new PaymentException(
        'Can\'t handle confirmation request without a valid transaction ID.');
    }

    $query = new EntityFieldQuery();
    $result = $query->entityCondition('entity_type', 'payment')
      ->propertyCondition('mpay24_tid', $_GET['TID'])
      ->execute();

    if (count($result['payment']) != 1) {
      throw new PaymentException(
        'Can\'t find matching payment object for this transaction ID.');
    }

    $payments = entity_load('payment', array_keys($result['payment']));
    $payment = array_shift($payments);

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

    db_insert('mpay24_payment_confirmation_interface')
      ->fields(array(
          'mpay24_tid'=> $request['TID'],
          'status'    => $request['STATUS'],
          'mpaytid'   => $request['MPAYTID'],
          'timestamp' => REQUEST_TIME,
        )
      )->execute();
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
    drupal_add_http_header('Content-Type', 'text/plain');
    print 'ERROR: ' . $e->message;
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

  drupal_add_js(
    array(
      'mpay24_payment' => array(
        'pid' => $payment->pid,
        'success_url'  => $success_url,
        //'error_url' => PAYMENT_MPAY24_ERROR_URL,
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
  $result = db_select('mpay24_payment_confirmation_interface', 's')
    ->fields('s', array('status', 'timestamp'))
    ->condition('tid', $payment->tid)
    ->orderBy('timestamp', 'DESC')
    ->range(0, 1)
    ->execute()
    ->fetchAssoc();

  $timeout = $payment->method->controller_data['request_timeout'];
  if (($result['timestamp'] + $timeout) < time()) {
    $result['status'] = 'TIMEOUT';
  }

  switch ($result['status']) {
    case 'ERROR':
    case 'SUSPENDED':
    case 'TIMEOUT':
      drupal_json_output(array('status' => 'error'));
      break;
    case 'BILLED':
    case 'CREDITED':
      drupal_json_output(array('status' => 'success'));
      break;
    default:
      drupal_json_output(array('status' => NULL));
  }
  exit(0);
}
