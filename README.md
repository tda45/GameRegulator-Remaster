# GameRegulator Remaster

Windows için geliştirilmiş oyun kısayolu düzenleme uygulaması.

## Özellikler

- Seçilen dizindeki oyun kısayollarını (.lnk) otomatik tespit eder
- Modern ve kullanıcı dostu arayüz
- Kısayolları masaüstündeki 'Oyunlar' klasörüne taşıma/kopyalama
- İşlem ilerlemesini gösteren progress bar
- Çoklu seçim desteği

## Teknik Gereksinimler

- **C++17**
- **Qt6 Framework**
- **Windows API** (lnk dosyalarını okumak için)
- **CMake 3.16+**

## Kurulum ve Derleme

### Gerekli Bağımlılıklar

1. **Qt6**'yı yükleyin:
   ```bash
   # Qt Online Installer kullanarak veya
   # vcpkg ile:
   vcpkg install qt6:x64-windows
   ```

2. **CMake** (3.16 veya üzeri)

3. **Visual Studio 2019+** (Windows için)

### Derleme Adımları

1. Proje klasörüne gidin:
   ```bash
   cd "GameRegulator Remaster"
   ```

2. Build klasörü oluşturun:
   ```bash
   mkdir build
   cd build
   ```

3. CMake ile yapılandırın:
   ```bash
   cmake .. -G "Visual Studio 16 2019" -A x64
   # veya Qt6 vcpkg ile kullanıyorsanız:
   cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
   ```

4. Derleyin:
   ```bash
   cmake --build . --config Release
   ```
   
   **Not:** Derleme işlemi sırasında `windeployqt` aracı otomatik olarak çalışarak tüm gerekli Qt DLL'lerini ve bağımlılıkları `build/bin/Release/` klasörüne kopyalar. Bu sayede uygulama taşınabilir hale gelir.

5. Uygulamayı çalıştırın:
   ```bash
   bin/Release/GameRegulatorRemaster.exe
   ```

## Kullanım

1. **Klasör Seç**: Üstteki "Klasör Seç" butonuna tıklayarak oyun kısayollarının bulunduğu dizini seçin.

2. **Kısayolları Seç**: Program otomatik olarak .lnk uzantılı dosyaları listeleyecektir. İstediğiniz kısayolları işaretleyin.

3. **Düzenle**: "Kısayolları Düzenle" butonuna tıklayarak seçili kısayolları masaüstündeki 'Oyunlar' klasörüne taşıyın.

## Dosya Yapısı

```
GameRegulator Remaster/
├── CMakeLists.txt          # CMake yapılandırma dosyası
├── main.cpp               # Ana uygulama giriş noktası
├── mainwindow.h           # Ana pencere sınıfı başlığı
├── mainwindow.cpp         # Ana pencere sınıfı implementasyonu
├── filemanager.h          # Dosya işlemleri sınıfı başlığı
├── filemanager.cpp        # Dosya işlemleri sınıfı implementasyonu
└── README.md              # Bu dosya
```

## Otomatik Dağıtım

Proje, `windeployqt` aracını kullanarak otomatik olarak tüm gerekli bağımlılıkları derleme sonrası kopyalar:

- **Qt DLL'leri**: Qt6Core, Qt6Gui, Qt6Widgets, Qt6Network, Qt6Svg
- **Platform eklentileri**: Windows platformu için gerekli eklentiler
- **Görüntü formatları**: JPEG, GIF, ICO, SVG desteği
- **Ağ ve TLS**: Ağ bağlantısı ve güvenlik için gerekli kütüphaneler
- **Diğer bağımlılıklar**: DirectX compiler, ICU kütüphaneleri

Bu özellik sayesinde derlenmiş uygulama taşınabilir hale gelir ve Qt kurulumu gerektirmez.

## Önemli Notlar

- Uygulama sadece Windows üzerinde çalışır
- .lnk dosyalarını okumak için Windows COM API kullanır
- Masaüstünde 'Oyunlar' klasörü otomatik oluşturulur
- Aynı isimli dosyalar için otomatik olarak benzersiz isimler oluşturulur
- Tüm Qt bağımlılıkları otomatik olarak exe ile birlikte kopyalanır

## Lisans

Bu proje özel kullanım için geliştirilmiştir.
