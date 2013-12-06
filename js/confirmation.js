jQuery(document).ready(function() {
    mpay24_payment = {
        poll: function () {
            jQuery.ajax({
                contentType: 'application/json',
                url: Drupal.settings.mpay24_payment.status_url,
                success: function(data) {
                    console.log('status', data.status);
                    if (data.status === 'poll') {
                        setTimeout(window.mpay24_payment.poll, 1000);
                    }
                    else if (data.status === 'redirect') {
                        window.mpay24_payment.redirect(data.url);
                    } else {
                        window.mpay24_payment.error('server error');
                    }
                },
                error: function(data) {
                    window.mpay24_payment.error('failed http request');
                },
            });
        },

        redirect: function(uri) {
            window.location.replace(uri)
        },

        init: function() {
            this.poll();
        }
    };
    window.mpay24_payment = mpay24_payment;
    mpay24_payment.init();
});
