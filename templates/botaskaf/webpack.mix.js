// SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
// SPDX-License-Identifier: AGPL-3.0-or-later

let mix = require('laravel-mix');

mix.options({
    processCssUrls: false,
    terser: {
        terserOptions: {
            output: {
                comments: /SPDX/i
            }
        }
    }
});

mix.sass('assets/sass/style.scss', 'static/css/');

mix.copy('node_modules/es-module-shims/dist/es-module-shims.js', 'static/js/');

mix.copyDirectory('node_modules/@popperjs/core/dist/esm', 'static/js/popper');

mix.copy('node_modules/bootstrap/js/index.esm.js', 'static/js/bootstrap');
mix.copy('node_modules/bootstrap/js/src', 'static/js/bootstrap/src');

// mix.copy('assets/js/index.js', 'static/js/').minify('static/js/index.js');
// mix.copy('assets/js/utils/icon.js', 'static/js/utils').minify('static/js/utils/icon.js');
// mix.copy('assets/js/utils/alerts.js', 'static/js/utils').minify('static/js/utils/alerts.js');
// mix.copy('assets/js/login/challenge.js', 'static/js/login').minify('static/js/login/challenge.js');
// mix.copy('assets/js/login/identifier.js', 'static/js/login').minify('static/js/login/identifier.js');

mix.copy('node_modules/bootstrap-icons/font/fonts', 'static/fonts');
