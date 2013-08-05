jQuery(document).ready(function() {
    mpay24_payment = {
        current_time_in_seconds: function () {
            return Math.floor(new Date().getTime() / 1000);
        },
        poll: function () {
            jQuery.ajax({
                contentType: 'application/json',
                url: "/mpay24_payment/confirm/status/"+Drupal.settings.mpay24_payment.pid,
                success: function(data) {
                    console.log('status', data.status);
                    if (data.status === null) {
                        setTimeout(function() { this.poll(); }, 1000);
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
        },
        init: function() {
            this.start = this.current_time_in_seconds();
            this.poll();
        }
    };
    window.mpay24_payment = mpay24_payment;
    mpay24_payment.init();
});
