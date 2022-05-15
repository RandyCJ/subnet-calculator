# Subnet calculator

**Instituto Tecnológico de Costa Rica**

**Tarea Corta 2**

**Redes**

**Número de Grupo: 20**

Elaborado por:  
Randy Conejo Juárez - 2019066448

Profesor:

Gerardo Nereo Campos Araya

Alajuela, 14 de mayo de 2022

## Instrucciones de ejecución

El proyecto fue implementado en un sistema Ubuntu 20.04, por lo que se
recomienda utilizar este sistema para replicar las instrucciones de
forma correcta.

Como primer paso instalamos Docker

`
  $ sudo apt install docker.io
`

Instalamos también docker-compose

`  $ sudo apt install docker-compose
`

También necesitamos instalar telnet

`
  $ sudo apt install telnet
`

Ahora entramos a la terminal y nos colocamos en la ruta del proyecto,
ingresamos como administrador.

  `
  $ sudo su
  `

Una vez ahí ejecutamos los siguientes dos comandos para construir y levantar el servicio de docker con la calculadora de subredes

  `
  $ docker-compose build
  `

  `
  $ docker-compose up
  `

Ahora necesitamos saber cual ip se le asignó a nuestro contenedor, para ello tenemos dos opciones, la primera es levantando un servicio llamado
portainer, que es una interfaz gráfica donde podemos manejar todas nuestras instancias de docker, esta la podemos poner a funcionar con el siguiente comando.

  ```
  sudo docker run -d -p 8000:8000 -p 9000:9000 \--name=portainer \--restart=always -v /var/run/docker.sock:/var/run/docker.sock portainer/portainer-ce
  ```

Abrimos nuestro navegador web de preferencia e ingresamos a
localhost:9000
![image19](https://user-images.githubusercontent.com/61055501/165863460-bfe00551-c8bd-4d07-a595-28ef80b109fa.png)



Aquí definimos una contraseña, tiene que ser de mínimo 8 caracteres.

Una vez definida ingresamos a Get Started
![image17](https://user-images.githubusercontent.com/61055501/165863502-8a45aa35-37c1-429f-ba34-c1655e7a7785.png)


Aquí seleccionamos local
![image20](https://user-images.githubusercontent.com/61055501/165863529-263b72fa-762b-4337-bdc6-d2aec2b3e78f.png)


Aquí nos saldrá el dashboard, donde podremos ver todos los contenedores
y redes que vayamos levantando.
![image6](https://user-images.githubusercontent.com/61055501/165863543-e679ead8-7061-4c7c-bbfa-f593008bce10.png)

Aqui seleccionamos 'containers', se nos deplegará la siguiente ventana, donde lo seleccionado en color verde es la IP del contenedor llamado 'subnet-calculator' que necesitamos.  
![ip_portainer](https://user-images.githubusercontent.com/61055501/168456342-07e7d4c0-8a27-4e23-b557-6e93459375c8.png)

Recomiendo esta opción, pues cada vez que levantemos nuevamente la máquina es probable que la IP cambie, por lo que aquí podremos ver facilmente la que necesitamos.

La otra opción es colocar en consola el siguiente comando

  `
  $ docker ps
  `

Esto nos desplegará la siguente información.  
![dockerps](https://user-images.githubusercontent.com/61055501/168456347-8e979058-e2ef-44bb-bd9b-57aef636787a.png)


Aquí copiamos el CONTAINER ID del contenedor subnet-calculator, que está seleccionado en verde, este puede variar según cada computadora.

Con dicho CONTAINER ID, colocamos el siguiente comando, colocando el container id que obtuvimos del comando anterior

  `
  $ docker inspect b84ba9f15a13
  `

Esto nos desplegará mucha información, lo que necesitamos que es la IP se encuentra al final en el campo "IPAddress".  
![IPCOMANDO](https://user-images.githubusercontent.com/61055501/168456352-270e4083-6627-4e93-b4b1-31416fd5d03e.png)


Una vez con nuestro IP conseguido de cualquiera de las dos formas, ingresamos desde una terminal a nuestro servidor mediante telnet, en la dirección IP
colocaremos la IP que conseguimos.

  `
  $ telnet 172.31.0.2 9666
  `

Una vez ahí, tendremos a nuestra disposición la calculadora de subredes para ejecutar nuestras consultas, basta con escribir nuestra consulta, pulsar la tecla enter, y el servidor analizará nuestro texto y retornará una respuesta.  
![telnet](https://user-images.githubusercontent.com/61055501/168456358-c562a033-e45e-4a9c-82a1-46a6aa61de43.png)



## Consultas disponibles

Las consultas que podemos realizar en la calculadora de subredes se detallan a continuación.

- GET BROADCAST IP {dirección IP} MASK {mascara en formato /bits o notación X.X.X.X}
    - Nos permite obtener el BROADCAST ADDRESS, ejemplos de uso
    - GET BROADCAST IP 10.8.2.5 MASK /29
    - GET BROADCAST IP 172.16.0.56 MASK 255.255.255.128
- GET NETWORK NUMBER IP {dirección IP} MASK {mascara en formato /bits o notación X.X.X.X}
    - Nos permite obtener el NETWORK ADDRESS, ejemplos de uso
    - GET NETWORK NUMBER IP 10.8.2.5 MASK /29
    - GET NETWORK NUMBER IP 172.16.0.56 MASK 255.255.255.128
- GET HOSTS RANGE IP {dirección IP} MASK {mascara en formato /bits o notación X.X.X.X}
    - Nos permite obtener el rango de direcciones IP válidas, ejemplos de uso
    - GET HOSTS RANGE IP 10.8.2.5 MASK /29
    - GET HOSTS RANGE IP 172.16.0.56 MASK 255.255.255.128
- GET RANDOM SUBNETS NETWORK NUMBER {Y.Y.Y.Y} MASK {mascara en formato /bits o notación X.X.X.X} NUMBER {número de redes} SIZE {mascara en formato /bits o notación X.X.X.X}
    - Nos permite obtener sub-redes aleatorias, ejemplos de uso
    - GET RANDOM SUBNETS NETWORK NUMBER 10.0.0.0 MASK /8 NUMBER 3 SIZE /24

## Ejemplos de uso, respuestas, y mensajes de error

Si escribimos algo que no concuerda con ninguna de las directivas descritas arriba, o los octetos tienen 4 o más números, o hay alguna letra donde no debería haber alguna, o si hay texto después de una directiva escrita correctamente, aunque sea un espacio, saldrá el siguiente error.  
![telnet1](https://user-images.githubusercontent.com/61055501/168456365-7873e969-2ac2-47fc-a9fc-f121e97f9a81.png)


Si escribimos las directivas bien, nos saldrá las respuestas a nuestros comandos.  
![telnet2](https://user-images.githubusercontent.com/61055501/168456368-4e48c5d4-5956-47bf-82cd-26b9e4f0fe58.png)


Los siguientes son los mensajes de error indicados por el servidor en caso de que hayamos escrito algo erroneo, se quiera realizar un cálculo imposible, o que algún parámetro no pueda tener un valor específico.  

En caso de ingresar en un octeto un número de tres dígitos mayor de 255, se nos desplegará este error.  
![telnet3](https://user-images.githubusercontent.com/61055501/168456370-becce4eb-13eb-495f-bdf7-4daaf682d5e8.png)


En caso de ingresar una máscara en formato /bits, esta deberá entre 8 y 32, de lo contrario se desplegará este error.  
![telnet4](https://user-images.githubusercontent.com/61055501/168456375-6279019e-af7f-4eca-b96b-5a3cc08676d5.png)

En caso de ingresar una máscara en formato (X.X.X.X), si está no concuerda con una máscara del formato /bits, como por ejemplo 255.255.255.1, se nos desplegará el siguiente error.  
![telnet5](https://user-images.githubusercontent.com/61055501/168456376-7a0e09f7-70bd-4310-8dce-122175e1b438.png)


En caso de usar la directiva GET RANDOM SUBNETS NETWORK, si ingresamos una máscara SIZE menor o igual que la máscara MASK, es imposible realizar un cálculo pues no es posible contener sub-redes en un máscara más grande, por lo que se nos desplegará el siguiente error.  
![telnet6](https://user-images.githubusercontent.com/61055501/168456381-c1d09852-e157-4497-9eab-d088a853a5c5.png)


En caso de usar la directiva GET RANDOM SUBNETS NETWORK, hay límites para la cantidad de sub-redes disponibles, por ejemplo, si colocamos un MASK de /8, y un SIZE de /9, solo habrán 2 posibles sub-redes con estos parámetros, si en NUMBER colocamos un número mayor que este no será posible satisfacer la consulta, por lo que se desplegará el siguiente mensaje.  
![telnet7](https://user-images.githubusercontent.com/61055501/168456384-23abbb5e-4acc-423c-801e-384a1b7e8aca.png)


En caso de usar la directiva GET HOSTS RANGE, no es posible obtener un rango de hosts de una máscara /bits mayor o igual a /31, pues con /31 solo hay dos posibles IPs, que corresponden al broadcast address y al network address y con /32 corresponde solo a la IP ingresada, por lo que se desplegará el siguiente error.  
![telnet8](https://user-images.githubusercontent.com/61055501/168456385-f76ccb09-cbe9-494a-bf6b-26383d5f8e23.png)

