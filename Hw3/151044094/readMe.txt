Denenmesi için içerisinde bütün ihtimaller olan bir directory
151044094 adlı klasörün içerisinde bulunmaktadır.

Hiçbir allocate olmaması rağmen; valgrindde onları freelememişim gibi görünüyor.

listdir v2.
- Artık file children proccessler parent ile pipe aracılığıyla iletişim kuruyor
- Artık directory children processler parent ile FIFO aracılığıyla iletişim kuruyor(ASIL İLETİŞİM)
- Directory proccess pipe ile o anki parent olan proccesse içerisinde kaç adet directory olduğunu iletiyor(YAN İLETİŞİM)

