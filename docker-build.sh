#Build `build` layer
#docker build -t hypercube --target build -f airflow/Dockerfile  .

#Force compilation of the hypercube app
#BUILD_APP=build_app
#docker build -t hypercube --target ${BUILD_APP} --no-cache-filter ${BUILD_APP} -f airflow/Dockerfile  .

#Build runtime image, with forced rebuild of the app
docker build -t hypercube --no-cache-filter ${BUILD_APP} -f airflow/Dockerfile  .
