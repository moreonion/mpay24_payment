jQuery(document).ready(function() {
    mpay24_payment = {
        current_time_in_seconds: function () {
            return Math.floor(new Date().getTime() / 1000);
        },

        poll: function () {
            jQuery.ajax({
                contentType: 'application/json',
                url: Drupal.settings.mpay24_payment.status_url + Drupal.settings.mpay24_payment.pid,
                success: function(data) {
                    console.log('status', data.status);
                    if (data.status === null) {
                        setTimeout(function() {
                            if (window.mpay24_payment.current_time_in_seconds() < window.mpay24_payment.timeout) {
                                window.mpay24_payment.poll();
                            } else {
                                window.mpay24_payment.error('timeout');
                            }
                        }, 1000);
                    }
                    else if (data.status === 'success') {
                        window.location.replace(Drupal.settings.mpay24_payment.success_url);
                    } else {
                        window.mpay24_payment.error('server error');
                    }
                },
                error: function(data) {
                    window.mpay24_payment.error('failed http request');
                },
            });
        },

        error: function(reason) {
            var s = Drupal.settings.mpay24_payment;
            window.location.replace(s.error_url + s.pid);
        },

        init: function() {
            this.timeout = this.current_time_in_seconds() + Drupal.settings.mpay24_payment.timeout;
            this.poll();
        }
    };
    window.mpay24_payment = mpay24_payment;
    mpay24_payment.init();
});
