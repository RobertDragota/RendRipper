# RendRipper

RendRipper este o aplicație C++20 pentru vizualizarea, secționarea și previzualizarea modelelor 3D. Integrează un motor de randare personalizat folosind OpenGL, o interfață bazată pe ImGui și unelte utile pentru generarea și tăierea modelelor.

## Caracteristici

- Încarcă fișiere `.obj` sau `.stl` și manipulează-le cu gizmo-uri de translație, rotație și scalare.
- Generează un mesh dintr-o singură imagine prin pipeline-ul [TripoSR](https://github.com/VAST-AI-Research/TripoSR) (neinclus în configurația actuală).
- Taie modelele folosind `CuraEngine` (neinclusă) împreună cu profilele pentru imprimante BambuLab și vizualizează G-code-ul rezultat strat cu strat.
- Previzualizează liniile de G-code în timp real cu OpenGL.

Managerul UI oferă ajutoare pentru aceste sarcini, inclusiv generarea modelului, procesul de tăiere și interacțiunea cu viewport-ul. Fișierele G-code sunt transformate în segmente de linii colorate pentru ca fiecare strat să poată fi desenat individual.

## Compilare

Acest proiect folosește CMake. Clonează depozitul împreună cu submodulele sale:

```bash
git clone --recurse-submodules <repo-url>
cd RendRipper
mkdir build && cd build
cmake ..
cmake --build .
```

Compilarea necesită pachete externe precum **assimp**, **nlohmann_json**, **curl** și un mediu de dezvoltare OpenGL. Pe Windows, căile către `CuraEngine.exe` și interpretul Python pentru TripoSR se definesc în `CMakeLists.txt` prin definiții de compilare.

**Notă:** depozitul nu conține copii locale ale [CuraEngine](https://github.com/Ultimaker/CuraEngine) și [TripoSR](https://github.com/VAST-AI-Research/TripoSR). Aceste proiecte trebuie descărcate separat pentru a permite funcțiile de tăiere și generare a modelelor.

## Rulare

După compilare, rulează executabilul `RendRipper`. Fereastra principală conține un viewport pentru randarea modelelor și meniuri pentru încărcarea modelelor, generarea mesh-urilor din imagini și pornirea procesului de tăiere.

Când tăierea s-a încheiat, G-code-ul generat poate fi previzualizat selectând stratul de afișat. Renderer-ul rulează până când fereastra este închisă de utilizator.

## Structura directoarelor

- `src/` – fișiere sursă C++ pentru renderer, UI și utilitare.
- `resources/` – shader-e, definiții de imprimante și exemple de G-code.
- `3DModelGenerator/TripoSR/` – cod Python pentru conversia imaginilor în modele 3D (nu este inclus implicit).
- `Slicer/` – folder destinat unui build `CuraEngine` (neinclus).
- `external/` – dependențe third-party aduse ca submodule git.

## Licență

Acest depozit nu conține momentan un fișier de licență la nivelul rădăcină. Consultați README-urile din subdirectoare pentru licențele codului terț inclus.
