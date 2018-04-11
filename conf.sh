#!/bin/bash

./configure \
  --prefix=/etc/nginx \
  --conf-path=/etc/nginx/nginx.conf \
  --sbin-path=/usr/bin/nginx \
  --pid-path=/run/nginx.pid \
  --lock-path=/run/lock/nginx.lock \
  --user=http \
  --group=http \
  --http-log-path=/var/log/nginx/access.log \
  --error-log-path=stderr \
  --http-client-body-temp-path=/var/lib/nginx/client-body \
  --http-proxy-temp-path=/var/lib/nginx/proxy \
  --http-fastcgi-temp-path=/var/lib/nginx/fastcgi \
  --http-scgi-temp-path=/var/lib/nginx/scgi \
  --http-uwsgi-temp-path=/var/lib/nginx/uwsgi \
  --with-compat \
  --with-debug \
  --with-file-aio \
  --with-http_addition_module \
  --with-http_auth_request_module \
  --with-http_dav_module \
  --with-http_degradation_module \
  --with-http_flv_module \
  --with-http_geoip_module \
  --with-http_gunzip_module \
  --with-http_gzip_static_module \
  --with-http_mp4_module \
  --with-http_realip_module \
  --with-http_secure_link_module \
  --with-http_slice_module \
  --with-http_ssl_module \
  --with-http_stub_status_module \
  --with-http_sub_module \
  --with-http_v2_module \
  --with-mail \
  --with-mail_ssl_module \
  --with-pcre-jit \
  --with-stream \
  --with-stream_realip_module \
  --with-stream_ssl_module \
  --with-stream_ssl_preread_module \
  --with-threads \
  --with-rina

#  --with-stream_geoip_module \
