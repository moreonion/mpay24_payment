jQuery(document).ready(function() {
    function poll_confirmation_interface() {
        console.log(Drupal.settings.mpay24_payment.pid);
        jQuery.ajax({
            contentType: 'application/json',
            url: "/mpay24_payment/confirm/status/"+Drupal.settings.mpay24_payment.pid,
            success: function(data) {
                if (data.status === null) {
                    setTimeout(function() { poll_confirmation_interface(); }, 1000);
                }
                else if (data.status === 'success') {
                    window.location.replace(Drupal.settings.mpay24_payment.success_url);
                } else {
                    //window.location.replace("/" + Drupal.settings.payment_mpay24.error_url +
                    //"/" + Drupal.settings.payment_mpay24.pid);
                }
            },
            error: function(data) {
                //window.location.replace("/" + Drupal.settings.payment_mpay24.error_url +
                //"/" + Drupal.settings.payment_mpay24.pid);
            },
        });
    }
    poll_confirmation_interface();
});
