# SheepDogWolf для macOS

Личная нативная сборка для Mac с Apple Silicon. Графика работает через Metal, звук — через CoreAudio.

- Русские меню, подсказки и диалоги; оригинальная английская озвучка.
- Масштаб рендеринга до 200%, сглаживание MSAA и фильтрация текстур.
- Игровые ресурсы берутся из собственной локальной копии игры. В Git они не хранятся.
- Личная `.app` содержит ресурсы внутри себя и упаковывается в `.dmg`.

## Сборка

Нужны Command Line Tools, CMake 3.20+, Python 3 и Pillow для создания русского шрифта.

```sh
cmake -S . -B build-mac -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 -DSDW_REPOSITORY= -DSDW_VERSION=1.1.3-mac.1
cmake --build build-mac --target SheepLauncher sdw_langexport resource_validation_tests jpeg_validation_tests -j 6
```

Зависимости загружаются при конфигурации; их версии и хеши закреплены в CMake. Можно передать локальные каталоги через `FETCHCONTENT_SOURCE_DIR_SDL3` и `FETCHCONTENT_SOURCE_DIR_IMGUI`.

Экспортируйте основу языкового пакета в ещё не существующую папку и создайте русский пакет. Подставьте собственные абсолютные пути:

```sh
build-mac/sdw_langexport /path/to/GameData /path/to/exported-font
python3 tools/build_russian_pack.py /path/to/exported-font /path/to/GameData/Languages/01-Russian --font '/System/Library/Fonts/Supplemental/Arial Narrow Bold.ttf'
```

`tools/package_macos.py` собирает личное приложение, включает лицензии и подписывает его локальной подписью. Параметры: `--app`, `--data`, `--sdl`, `--imgui`, `--output`. Выходная папка должна быть новой.

Для DMG поместите приложение и ссылку на `/Applications` в отдельную папку, затем выполните:

```sh
hdiutil create -volname SheepDogWolf -srcfolder /path/to/staging -format UDZO -ov /path/to/SheepDogWolf-macOS-arm64.dmg
hdiutil verify /path/to/SheepDogWolf-macOS-arm64.dmg
```

## Проверка

`resource_validation_tests` принимает пути к `.WAR` и `.DAV`; `jpeg_validation_tests` — пути к галереям `.sdw`. `tools/smoke_levels.py GAME DATA OUTPUT` проверяет загрузку всех игровых уровней и основных сцен с Metal. Это проверка запуска, а не полное прохождение игры.

Сохранения находятся в `~/Library/Application Support/Infogrames/Sheep, Dog'n Wolf DX7`. Для изолированных тестов предусмотрена переменная `SDW_SAVE_DIR`.

## Лицензии

Обязательные условия использования, авторские уведомления и происхождение компонентов сохранены в [LICENSE](LICENSE), [NOTICE.md](NOTICE.md), `licenses/` и разделе «О программе». Права на исходную игру и её ресурсы принадлежат их правообладателям.

Задачи и ход выполнения: [docs/TASKS_RU.txt](docs/TASKS_RU.txt). Подробный статус проверок: [docs/MACOS_STATUS_RU.txt](docs/MACOS_STATUS_RU.txt).
