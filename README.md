# 3DAnimApp
Aplikacja do projektowania animowanych scen 3D zrealizowana w ramach pracy inżynierskiej // Application for designing animated 3D scenes developed for bachelor's thesis


## Funkcje aplikacji

- załadowanie siatki obiektu z pliku *.obj (lista trójkątów indeksowych)
- przesuwanie, rotacja, skalowanie i zmiana koloru powierzchni obiektów w scenie
- dodawanie i manipulacja wirtualnych kamer i źródeł światła w scenie
- animacja elementów sceny poprzez zmianę ich parametrów w klatkach kluczowych
- przesuwanie klatek kluczowych zwizualizowanych na osi czasu
- ustalanie cięć między kamerami poprzez wskazanie indeksu kamery docelowej
- renderowanie projektowanej sceny w czasie rzeczywistym
- tworzenie wielu scen i przycinanie ich oraz ustawianie w sekwencji
- zapisywanie i wczytywanie sceny do/z pliku *.txt
- eksportowanie sekwencji scen do pliku *.avi


## Obsługa aplikacji

W aktualnej wersji do działania plikacja wymaga plików 3DAnimApp.cpp, 3DAnimApp.h i font.txt oraz biblioteki OpenCV. Pliki *.obj z siatkami obiektów, które mają być 
załadowane w aplikacji, muszą być umieszczone w folderze razem z pozostałymi plikami aplikacji, a ich nazwy muszą być wpisane do kolejnych wierszy pliku objects.txt.

Po uruchomieniu aplikacji wyświetlony zostaje interfejs graficzny w trybie edycji sekwencji scen, gdzie możliwe jest przełączanie wybranej sceny (UP/DOWN),
jej przycinanie (ALT + LEFT/RIGHT), zmiana jej kolejności w sekwencji (ALT + UP/DOWN) oraz ustalenie koloru jej etykiety ('C' + UP/DOWN/LEFT/RIGHT),
a także dodawanie scen (L SHIFT) i ich usuwanie (DELETE). W celu edycji zawartości wybranej sceny konieczne jest przejście do trybu edycji sceny (ENTER).

Niezależnie od trybu w którym znajduje się aplikacja w dany momencie, interfejs graficzny jest podzielony na sześć okien, których zawartość zmienia się 
zależnie od aktualnie wykonywanej funkcji. Wyróżnione zostały okna nawigacji (1), podglądu (2), osi czasu (3), zawartości (4), zarządzania (5) i edycji (6).


<img width="1348" height="1078" alt="Image" src="https://github.com/user-attachments/assets/97f85feb-e12d-4892-be20-80be7b32a015" width="8"/>


W trybie edycji sceny w oknie zawartości przedstawiona jest lista elementów wybranego rodzaju - obiektów ('O'), kamer ('P') lub źródeł światła ('L').
W oknie zarządzania wyświetlone są dostępne opcje - wybór elementu z listy (UP/DOWN), jego usunięcie (DELETE) oraz dodanie nowego (L SHIFT). W przypadku dodawania
nowego obiektu, konieczne jest podanie indeksu wskazującego plik z siatką obiektu. Lista dostępnych plików *.obj, zaimportowana z pliku tekstowego objects.txt, 
zostaje wyświetlona w oknie edycji.

Zmiana parametrów wybranego elementu realizowana jest poprzez modyfikację klatki kluczowej. Każdy element domyślnie posiada co najmniej jedną klatkę kluczową, 
możliwe jest też ich dodawanie (R SHIFT), usuwanie (R CTRL) oraz przełączanie (LEFT/RIGHT). Jeśli w aktualnym punkcie na osi czasu dany element posiada klatkę kluczową, 
w oknie edycji wyświetlone zostają dostępne możliwości transformacji elmentu. W drugiej zakładce tego okna znajdują się opcje przesunięcia klatki kluczowej 
w czasie (ALT + LEFT/RIGHT), a w przypadku edycji klatki kluczowej kamery również dodanie cięcia do innej kamery poprzez wskazanie jej indeksu (ALT + nr kamery).

W ramach transformacji obiektu w klatce kluczowej możliwa jest zmiana jego pozycji ('W'/'S'/'A'/'D'/'Q'/'E'), rotacji ('U'/'J'/'H'/'K'/'Y'/'I') oraz skali ('+','-').
Podczas edycji klatki kluczowej kamery nie jest możliwe jej skalowanie. W przypadku źrodeł światła rotacja nie jest dostępna, a skalowanie jest tożsame 
ze zmianą jego intensywności. Możliwa jest także zmiana koloru każdego wybranego elementu ('C' + UP/DOWN/LEFT/RIGHT) na podstawie koła HSV wyświetlanego w oknie edycji.

W oknie podglądu wyświetlany jest renderowany w czasie rzeczywistym podgląd projektowanej sceny z perspektywy aktualnie wybranej kamery, lub wolnej kamery (TAB).
W oknie nawigacji przedstawiony jest aktualny czas wybranej sceny oraz indeks wybranej kamery, a w trybie edycji sekwencji indeks wybranej sceny.
Zarówno w trybie edycji sceny, jak i sekwencji scen możliwe jest odtworzenie/zatrzymanie zaprojektowanej animacji (SPACE) oraz powrót do jej początku (BACKSPACE).

Po powrocie do trybu edycji sekwencji scen (ESC) możliwy jest eksport utworzonej animacji do pliku OutputAnimation.avi (L CTRL + 'E'). Ponadto, w każdym momencie 
zaprojektowana sekwencja scen może być zapisana do pliku data.txt (L CTRL + 'Z'), po czym można bezpiecznie zamknąć aplikację. Przy ponownym uruchomieniu aplikacji 
zapisana sekwencja scen zostanie automatycznie odtworzona.
