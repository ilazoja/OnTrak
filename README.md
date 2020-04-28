# OnTrak

Going Left

![Going Left](GoingLeft.gif)

Going Right

![Going Right](GoingRight.gif)

Going Straight

![Going Straight](GoingStraight.gif)

Obstacle

![Obstacle](Obstacle.gif)


## How to get it to work on your Android Studio

In build.gradle for the app, add 

android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                abiFilters 'arm64-v8a', 'armeabi-v7a'
            }
        }
    }
}

In CMakeLists.txt change the paths to where they are on your computer.
