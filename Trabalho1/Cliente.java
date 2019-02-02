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


class Cliente {
    public static void main( String args[] ) throws IOException {
        //DeclaraÃ§Ã£o das variÃ¡veis 
        int porta;
        String ip;
        Socket socket;
        
        try {
            ip = "localhost";
            porta = 4321;
            
            File f = new File("C:\\Users\\Mauricio\\Documents\\NetBeansProjects\\ProgDist1\\Teste.txt");
            FileInputStream in = new FileInputStream(f);
            
            socket = new Socket();
            //Vinculando o endereÃ§o ao socket e realizando a conexÃ£o
            InetSocketAddress endereco = new InetSocketAddress(ip, porta);
            socket.connect(endereco, 1000);//Teste de tempo caso nÃ£o seja estabelecida a conexÃ£o
            //Enviar e receber (servidor e cliente)
            
            
            
            int filesize=6022386;
            int bytesRead;
            int current = 0;
            // recebendo o arquivo
            byte [] mybytearray  = new byte [filesize];
            InputStream is = socket.getInputStream();
            FileOutputStream fos = new FileOutputStream("Teste-saida.txt");
            BufferedOutputStream bos = new BufferedOutputStream(fos);
            bytesRead = is.read(mybytearray,0,mybytearray.length);
            current = bytesRead;
            do {
                bytesRead = is.read(mybytearray, current, (mybytearray.length-current));
                if(bytesRead >= 0) current += bytesRead;
                    } while(bytesRead > -1);
                bos.write(mybytearray, 0 , current);
                bos.close();
            
            
            

            socket.close();
        } 
        catch (IOException e) {
            System.out.println(e);
        }
    }
}

