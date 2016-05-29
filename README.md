# FTP-Server

HASAN MEN - 131044009
Sistem Programlama Final Projesi
29.5.2016
MultiClient-Server Socket programming


##_Main-Server Nasil Calisir_##

1. Socket baglantilari tamamlanir
2. MainServer olusacak miniserverler ile haberlesme icin 2 pipe ve bir thread olusturur.
3. Bu thread pipe a gelen istekleri yanitlayak.
4. MainServer accept durumuna gecer ve client baglaninca fork ile miniserver olusturur.

## _Main-Pipe threadi ne yapar?_##
1. MiniServerler yani childler pipe yazinca bu thread hemen okur.
2. Childler pipe dosya varmi yokmu, servere bagli client sayisi gibi bilgileri isterler.
3. Her bilgi aktarimi sirasinda SEMAFOR ile karismalar engellendi.
4. Sonuclar ise cevap pipe indan yazilir ve bu thread ol komutu alana kadar devam eder.

## _MiniServerler nasil calisirlar?_##
1. Client servere baglanir.
2. Server dosya aktarimlari icin bir FIFO ve karismamasi icinde semafor acar.
3. Clien servere komutu yollar. Socket  okuma icin kilitlenir.
  1.lsClient komutu : server pipe uzerinden main servere sorar ve listeyi alir. Daha sonra
  pipe uzerinden gelen bilgiler socket yardimiyla cliente yollanir.
  2.listServer : server icindeki dosyalar hemen okunur ve socket ile yollanir.
  3.sendFile : kime gidecegi ile birlikte bir dosya gelir. Eger gidecek kisi yoksa servere gider dosya.
  AYNI ANDA AYNI ISIMLI DOSYALAR GELIRSE ILK DOSYA SEMAFOR ACAR VE KENDINI KILITLER VE BITER SONRA 2.SI
  AYNI SEKILDE KOPYALANIR. YANI EN SON KIM YOLLARSA ONUNKI KAYDEDILIR
  4.DIE : servere olmasi icin emir gelir. isler normal sekilde biter


## _Client Nasil calisir?_ ##
1. MainServere port ve ip uzerinden baglanir.
2. Kendisine ozel mini server acilir ve iletisim buradan devam eder.
3. Hemen bir thread acar ve socketten gelen bilgileri thread dinler.
4. Hem kullanicidan komut beklerken arka plandan dosyada alabilir.
5. Help manualinde komutlar belirtildi.
  1.listLocal : bulundugu konumdaki dosyalari listeler
  2.sendFile : servere gonderilen dosyalar degisieme ugramadan gider.
  3. client-clent arasi dosya aktariminde dosyanin sonuna -new takisi gelir.
  4.listServer : sockete komut gider ve thread cevabi okuyarak serverdeki dosyalari listeler


###_Sinyaller_###
1. Clienlere verilen sinyaller handle edildi.
2. Servere kill gelince clienti oldurmuyor suan ama kendisi oluyor. Client ayrica oldurulmeli.
3. Tum bunlara ek olarak herhangi bir leak yok.
