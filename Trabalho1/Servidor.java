/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package progdist1;

/**
 *
 * @author Mauricio
 */

import java.net.*;
import java.io.*;

class Servidor{
    public static void main( String args[] ) {
        //Declaração das variáveis
        ServerSocket servico = (ServerSocket)null;
        Socket socket;
        //DataOutputStream enviar;
        
        try {
            //Criar o ServerSocket, espear a conexão na porta 4321
            servico = new ServerSocket(4321);
        } 
        catch( IOException e ) {
            //Exceção
            System.out.println( e );
        }

        while(true) {
            try {
                //Criando objeto socket para tratar a conexão com o cliente, assim
                //que o pedido chegar da conexão ao servidor
                socket = servico.accept();
                //Criação do objeto do servidor Thread para execução
            
                File myFile = new File ("Teste.txt");
                byte [] mybytearray  = new byte [(int)myFile.length()];
                FileInputStream fis = new FileInputStream(myFile);
                BufferedInputStream bis = new BufferedInputStream(fis);
                bis.read(mybytearray,0,mybytearray.length);
                OutputStream os = socket.getOutputStream();
                System.out.println("Enviando...");
                os.write(mybytearray,0,mybytearray.length);
                os.flush();
                socket.close();
            } 
            catch( IOException e ){
                System.out.println( e );
            }
        }
    }
}
