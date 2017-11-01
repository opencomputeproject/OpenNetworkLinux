# How to build ONL behind an HTTP Proxy

Many corporate environments don't provide native access to the Internet
and instead all access must go through an HTTP proxy.  Since the ONL
build process dynamically pulls lots of things, this can be a pain.
While everyone's setup is different, hopefully these directions help
reduce that pain.


1) Update the git modules to point to http: instead of git:

    sed -i -e 's/git:/http:/' $ONL/.gitmodules

2) Make sure you have apt-cacher-ng installed in your host (non-docker)
    environment and that docker starts it.  Next, configure it to use
    your proxy:

     $ grep Proxy /etc/apt-cacher-ng/acng.conf
     Proxy: http://myproxy.mycompany.com:8080
     $ sudo /etc/init.d/apt-cacher-ng restart

3) Make sure your git config is configured correctly for 
    proxies:

    $ cat ~/.gitconfig
    [https]
        proxy = myproxy.mycompany.com:8080
    [https]
        proxy = myproxy.mycompany.com:8080
