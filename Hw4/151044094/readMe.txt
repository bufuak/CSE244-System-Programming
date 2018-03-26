Bu ödevde 3 adet struct yapısı tanımlanmış ve fonksiyonların return değeri olarak kullanılmıştır.

1) ListResult- Bir file okunduğunda(list programı tarafından) onun toplamda kaç satır okuduğunu ve kaç tane string bulduğunu döndürmek için kullanılır

2) ThreadResult- Thread oluşturulurken ona, logPtr,paths,tid,numberOfString,numberOfLine,numberOfFiles argümanlarının aktarılmasını ve
numberOfStrings,numberOfLines değişkenlerinin ana process'e geri ulaştırılmasını sağlar.

3) Result- Gerçek sonucu saklayan struct. ThreadResult'un değerlerini de içine katarak, toplam kaç file,directory dolaştığını, kaç thread oluşturduğunu,
toplam en fazla kaç tane thread'in oluşturulduğunu saklar. Main programına ulaştırır.

Program gelen CTRL+C ve CTRL+Z sinyallerini blocklar, işlemine devam eder. Ancak kullanıcıya terminalde bu kombinasyonlardan birinin basıldığının bilgisini basar.

Processler arası iletişim pipe ile sağlanmıştır.

Thread counter tutarken mutex kullanmıştır.

Thread havuzu o klasördeki file sayısı kadar threadle doldurulur.(malloc) O klasörün bütün işi bittiğinde bu alınan memory sisteme geri verilir.(free)
Valgrind raporunda ilk çıktıların o kadar az freelenmiş olma sebebi de budur. İlk rapor en içteki directory'nin sonucu olduğu için onun üstündeki
directory için açılmış olan threadler daha kapanmamıştır. Ancak en sondaki raporumuzda yani mainProcess'in raporunda sadece kapanmamış bir malloc görünmektedir.
Bu allocate, hiçbir malloc operasyonu kullanılmasa da görünmektedir.(bknz ikinci ödev valgrind sonucu:ScreenShot4) Sonuç olarak sistemden alınan bütün memory, sisteme geri verilmiştir.