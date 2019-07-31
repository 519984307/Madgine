

 def axisList = [
    ["clang-linux","clang-android","emscripten"],           //toolchain
    ["Debug","RelWithDebInfo"],      //configuration
    ["", "OpenGL"]                   //static configuration file
]   

def tasks = [:]
def comboBuilder
def comboEntry = []


def task = {
    // builds and returns the task for each combination

    /* Map the entries back to a more readable format
       the index will correspond to the position of this axis in axisList[] */
    def toolchain = it[0]
    def configuration = it[1]
    def staticConfig = it[2]
	
    def name = "${it.join('-')}"    

	if (staticConfig?.trim())
		staticConfig = "test/${staticConfig}_${toolchain}.cfg"

    return {
        // This is where the important work happens for each combination
	    stage ("${name}") {
            stage("cmake") {
			    sh """
				if ${params.fullBuild}; then
					if [ -d "${name}" ]; then 
						rm -Rf ${name};
					fi
				fi
				mkdir -p ${name}
				cd ${name}
			    cmake .. \
		        -DCMAKE_BUILD_TYPE=${configuration} \
		        -DCMAKE_TOOLCHAIN_FILE=~/toolchains/${toolchain}.cmake \
                -DSTATIC_BUILD=${staticConfig}
			    """
            }
            stage("build") {				
				sh """
				cd ${name}
				make all
				"""				
            }
		    docker.image('rikorose/gcc-cmake:3.10.3').inside {
				stage("Test") {
					sh """
					cd ${name}
					ctest
					"""
				}
            }           
        }
    }
}

/*
    This is where the magic happens
    recursively work through the axisList and build all combinations
*/
comboBuilder = { def axes, int level ->
    for ( entry in axes[0] ) {
        comboEntry[level] = entry
        if (axes.size() > 1 ) {
            comboBuilder(axes.drop(1), level + 1)
        }
        else {
            tasks[comboEntry.join("-")] = task(comboEntry.collect())
        }
    }
}

comboBuilder(axisList, 0)    

pipeline {
    agent any

	parameters {
        booleanParam(defaultValue: false, description: '', name: 'fullBuild')
    }

	options{
		ansiColor('xterm')
		disableConcurrentBuilds()
	}

    stages {
		stage("checkout") {
			steps{
        		checkout scm
				sh """
				git submodule update --init --recursive
				"""
			}
	    }
        stage ("Multiconfiguration Parallel Tasks") {
	        steps {
			    script {
				    parallel tasks
			    }
	        }
        }
    }

	post {
        always {
            junit '**/*.xml'
		
	    recordIssues enabledForFailure: true, tools: [clang()]
        }
    }
}
