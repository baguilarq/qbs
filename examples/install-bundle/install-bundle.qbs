import qbs
import qbs.FileInfo

Project {
    CppApplication {
        Depends { name: "windowutils" }
//        Depends { name: "ib"; condition: qbs.targetOS.contains("darwin") }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"] }

        name: "window"
//        property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
//        targetName:  "window"
        files: [
            "main.cpp",
            "assetcatalog1.xcassets",
            "assetcatalog2.xcassets",
            "white.iconset",
            "MainMenu.xib",
            "Storyboard.storyboard"
        ]

//        cpp.minimumMacosVersion: "10.10"

//        Group {
//            fileTagsFilter:  ["application"]
//            qbs.install: true
//            qbs.installDir: isBundle ? "Applications" : (qbs.targetOS.contains("windows") ? "" : "bin")
//            qbs.installDir: ""
//            qbs.installSourceBase: product.buildDirectory
//        }
    }

    DynamicLibrary {
        Depends { name: "cpp" }

        name: "windowutils"
//        property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
//        targetName: isBundle ? "WindowUtils" : "windowutils"
        files: ["coreutils.cpp", "coreutils.h"]

        Group {
//            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_symlink", "dynamiclibrary_import"]
//            qbs.install: true
//            qbs.installDir: ""
//            qbs.installSourceBase: product.buildDirectory
        }
    }
}
