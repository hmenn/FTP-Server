# FTP-Server


MultiClient-Server Socket programming


##Server Tarafı

-[x]Client Server bağlantısı yapıldı.
-[x]Her client için ayrı server fork ile açıldı.
-[x]MiniServerler ile iletişim pipe ile sağlanıyor ve pipe kontrolu ise thread
ile sağlanıyor.


##Client Tarafı
-[x]Clintler istek sonucu bir socket fildes ile eşlenirler.
-[x]Bu sockete kendi pid lerini göndererek şeleşmeyi sağlarlar.
-[x]Socketten okuma kısmını bir thread üstlenir ve okuduğu bilgileri
duruma göre yönlendirir.
-[x]help-listlocal-lsclient komutları yazildı.
-[ ]listserver ve send file yazilacak

####_Sinyaller_####
-Sinyal handle kısımlarında eksikler var.
-[ ]Sinyal yapısı daha detaylı tasarlanacak.