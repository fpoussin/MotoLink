pipeline {
  agent any
  stages {
    stage('Prepare code') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-18.04-arm'
        }

      }
      steps {
        sh '''git submodule sync
git submodule update --init'''
      }
    }
    stage('Compile uC Bootloader') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-18.04-arm'
        }

      }
      steps {
        sh '''cd $WORKSPACE/code/bootloader
make clean
nice make -j $(nproc)'''
      }
    }
    stage('Compile uC Application') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-18.04-arm'
        }

      }
      steps {
        sh '''cd $WORKSPACE/code/app/dsp_lib
make clean
nice make -j $(nproc)
cd ..
make clean
nice make -j $(nproc)
'''
      }
    }
    stage('Compile GUI') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-18.04-qt5'
        }

      }
      steps {
        sh '''cd $WORKSPACE/GUI/res
unzip oxygen.zip
touch firmware.xml'''
        sh '''cd $WORKSPACE/GUI
qmake
nice make -j $(nproc)
'''
      }
    }
  }
}