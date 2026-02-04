# 3DAnimApp
Aplikacja do projektowania animowanych scen 3D zrealizowana w ramach pracy inżynierskiej // Application for designing animated 3D scenes developed for bachelor's thesis

*ENGLISH BELOW*

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

W aktualnej wersji do działania plikacja wymaga plików 3DAnimApp.cpp, 3DAnimApp.h i font.txt oraz biblioteki OpenCV. Pliki *.obj z siatkami obiektów, które mają być załadowane w aplikacji, muszą być umieszczone w folderze razem z pozostałymi plikami aplikacji, a ich nazwy muszą być wpisane do kolejnych wierszy pliku objects.txt.

Po uruchomieniu aplikacji wyświetlony zostaje interfejs graficzny w trybie edycji sekwencji scen, gdzie możliwe jest przełączanie wybranej sceny (UP/DOWN), jej przycinanie (ALT + LEFT/RIGHT), zmiana jej kolejności w sekwencji (ALT + UP/DOWN) oraz ustalenie koloru jej etykiety ('C' + UP/DOWN/LEFT/RIGHT), a także dodawanie scen (L SHIFT) i ich usuwanie (DELETE). W celu edycji zawartości wybranej sceny konieczne jest przejście do trybu edycji sceny (ENTER).

Niezależnie od trybu w którym znajduje się aplikacja w dany momencie, interfejs graficzny jest podzielony na sześć okien, których zawartość zmienia się zależnie od aktualnie wykonywanej funkcji. Wyróżnione zostały okna nawigacji (1), podglądu (2), osi czasu (3), zawartości (4), zarządzania (5) i edycji (6).


<center><img width="774" height="537" alt="Image" src="https://github.com/user-attachments/assets/97f85feb-e12d-4892-be20-80be7b32a015"></center>


W trybie edycji sceny w oknie zawartości przedstawiona jest lista elementów wybranego rodzaju - obiektów ('O'), kamer ('P') lub źródeł światła ('L'). W oknie zarządzania wyświetlone są dostępne opcje - wybór elementu z listy (UP/DOWN), jego usunięcie (DELETE) oraz dodanie nowego (L SHIFT). W przypadku dodawania nowego obiektu, konieczne jest podanie indeksu wskazującego plik z siatką obiektu. Lista dostępnych plików *.obj, zaimportowana z pliku tekstowego objects.txt, zostaje wyświetlona w oknie edycji.

Zmiana parametrów wybranego elementu realizowana jest poprzez modyfikację klatki kluczowej. Każdy element domyślnie posiada co najmniej jedną klatkę kluczową, możliwe jest też ich dodawanie (R SHIFT), usuwanie (R CTRL) oraz przełączanie (LEFT/RIGHT). Jeśli w aktualnym punkcie na osi czasu dany element posiada klatkę kluczową, w oknie edycji wyświetlone zostają dostępne możliwości transformacji elmentu. W drugiej zakładce tego okna znajdują się opcje przesunięcia klatki kluczowej w czasie (ALT + LEFT/RIGHT), a w przypadku edycji klatki kluczowej kamery również dodanie cięcia do innej kamery poprzez wskazanie jej indeksu (ALT + nr kamery).

W ramach transformacji obiektu w klatce kluczowej możliwa jest zmiana jego pozycji ('W'/'S'/'A'/'D'/'Q'/'E'), rotacji ('U'/'J'/'H'/'K'/'Y'/'I') oraz skali ('+','-'). Podczas edycji klatki kluczowej kamery nie jest możliwe jej skalowanie. W przypadku źrodeł światła rotacja nie jest dostępna, a skalowanie jest tożsame ze zmianą jego intensywności. Możliwa jest także zmiana koloru każdego wybranego elementu ('C' + UP/DOWN/LEFT/RIGHT) na podstawie koła HSV wyświetlanego w oknie edycji.

W oknie podglądu wyświetlany jest renderowany w czasie rzeczywistym podgląd projektowanej sceny z perspektywy aktualnie wybranej kamery, lub wolnej kamery (TAB). W oknie nawigacji przedstawiony jest aktualny czas wybranej sceny oraz indeks wybranej kamery, a w trybie edycji sekwencji indeks wybranej sceny. Zarówno w trybie edycji sceny, jak i sekwencji scen możliwe jest odtworzenie/zatrzymanie zaprojektowanej animacji (SPACE) oraz powrót do jej początku (BACKSPACE).

Po powrocie do trybu edycji sekwencji scen (ESC) możliwy jest eksport utworzonej animacji do pliku OutputAnimation.avi (L CTRL + 'E'). Ponadto, w każdym momencie zaprojektowana sekwencja scen może być zapisana do pliku data.txt (L CTRL + 'Z'), po czym można bezpiecznie zamknąć aplikację (ESC). Przy ponownym uruchomieniu aplikacji zapisana sekwencja scen zostanie automatycznie odtworzona.


-------------------

## Features

- loading *.obj 3D meshes (indexed triangle list)
- positioning, rotation, scaling and changing color of any object in a scene
- adding and manipulating multiple cameras and light sources in a scene
- animating scene elemets by adding new keyframes and modifying properities
- manipulating keyframes with visualization on the timeline
- cutting between cameras by pointing to target camera index in a keyframe
- real time scene visualization with playback
- sequencing multiple scenes into complex animations
- saving and loading scene sequence to/from a *.txt file
- exporting scene sequence to an *.avi file


## Usage

To work properly, the current version of this application requires the 3DAnimApp.cpp, 3DAnimApp.h, font.txt files and the OpenCV library. All the *.obj files containing 3D meshes which are to be loaded need to be located in the same directory as the other app files, and their filenames should be listed in the objects.txt file.

After running the application it enters the sequence edit mode, also displayed by the GUI. It allows the user to select a scene from a list (UP/DOWN), trim it (ALT + LEFT/RIGHT), move it in a sequence (ALT + UP/DOWN) and change its label's color ('C' + UP/DOWN/LEFT/RIGHT). A new scene can also be added (L SHIFT) and the selected scene can be deleted (DELETE). In order to edit the content of a selected scene, it is required to enter the scene edit mode (ENTER).

In both modes the GUI is divided into six windows, the content of which depends on the current mode and action. There is a navigation window (1), preview window (2), timeline window (3), content window (4), management window (5) and edit window (6).

<center><img width="774" height="537" alt="Image" src="https://github.com/user-attachments/assets/97f85feb-e12d-4892-be20-80be7b32a015"></center>

In the scene edit mode the content window presents a list of all the scene elements of a selected type - objects ('O'), cameras ('P') or light sources ('L'). The management window shows available options - selecting an element from the list (UP/DOWN), deleting it (DELETE) or adding a new one (L SHIFT). While adding a new object it is also necessary to input the index correspoding to the filename of the selected mesh. The list of available files, imported from objects.txt, is presented in the edit window.

Changing the parameters of a selected element is executed by modifying its keyframe. Each element always has at least one keyframe, it's also possible to add keyframes (R SHIFT), delete them (R CTRL) and select them (LEFT/RIGHT). If the selected element has a keyframe in the current point on the timeline, the edit window presents available transformation options. The other tab in the edit window contains options of moving the keyframe in time (ALT + LEFT/RIGHT), and additionally, if the selected element is a camera, an option to cut to another camera by inputting its index (ALT + cam index).

When transforming and object's keyframe it's possible to change its position ('W'/'S'/'A'/'D'/'Q'/'E'), rotation ('U'/'J'/'H'/'K'/'Y'/'I') and scale ('+','-'). It is not possible to change the scale of a camera or a rotation of a light source, and scaling a light source means changing it's intensity. It is also possible to change the color of the selected object ('C' + UP/DOWN/LEFT/RIGHT), with the help of the HSV color wheel, shown in the edit window.

The preview window presents a real time view of the 3D scene from the perspective of the selected camera, or the free camera (TAB). The navigation window shows the current time and the index of the selected camera, while in the sequence edit mode it also presents the index of the selected scene. In both modes it's also possible to playback the scene or the scene sequence (SPACE) and to rewind it to the start (BACKSPACE).

After returning to the sequence edit mode (ESC) the scene sequence can be exported to OutputAnimation.avi file (L CTRL + 'E'). Also, at any point in time it's possible to save the scene sequence to a data.txt file (L CTRL + 'Z') and safely quit the application (ESC). After it's run again, the app will automatically load the saved sequence.
