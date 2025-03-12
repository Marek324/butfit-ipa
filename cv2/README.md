
# Spuštění na linux (Debian like)


## Instalace potřebných balíčků

1. **Aktualizujte seznam balíčků:**
   Otevřete terminál a aktualizujte seznam balíčků, abyste měli nejnovější informace o balíčcích:
   ```bash
   sudo apt update
   ```

2. **Nainstalujte OpenCV a build tools:**
   Nainstalujte OpenCV vývojové knihovny, g++, a make pomocí následujícího příkazu:
   ```bash
   sudo apt install libopencv-dev build-essential
   ```
   Tento příkaz nainstaluje:
    * `libopencv-dev`:  OpenCV vývojové soubory, včetně hlavičkových souborů a knihoven potřebných pro kompilaci OpenCV programů.
    * `build-essential`:  Základní nástroje pro kompilaci softwaru, včetně `g++` a `make`.

    Může se stát, že se nepodaří najít OpenCV knihovnu. V tomto případě, musíte nastavit cesty v Makefile (komentáře k Option 1).

## Kompilace

1. **Přejděte do adresáře projektu cv2 **

2. **Zkompilujte kód - možnost Makefile**
   Spusťte příkaz `make` v terminálu:
   ```bash
   make
   ```

   V případě, že používate VS Code, spustě task (ctrl+shift+p) Run Tasks: Make Build.

## Spuštění

1. **Spuštění**
   ```bash
   ./cv2
   ```
   Toto spustí program, provede úpravu jasu pomocí různých metod (ASM, AVX2, MMX), změří časy provedení, a zobrazí výsledky v okně (pokud běžíte v grafickém prostředí). Výsledky benchmarků se také vypíší do terminálu.

## Poznámky

* **AVX2 a MMX Podpora:** Kód používá AVX2 a MMX instrukce pro optimalizované zpracování obrazu.
* **Grafické prostředí (Volitelné):**  Zobrazení výstupního obrázku pomocí `imshow` vyžaduje grafické prostředí. Pokud běžíte na serveru bez GUI, program se stále spustí a vypíše výsledky benchmarků do terminálu, ale okno s obrázkem se nezobrazí.
* **Výkon:** Výsledky benchmarků se mohou lišit v závislosti na vašem CPU, zatížení systému a dalších faktorech.
