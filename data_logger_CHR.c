//****************************************************************************
//* ADQUISICION DE DATOS DE CAMARA DE HUMEDAD 
//* PARA PRUEBAS DE HIDROGEL
//* DESARROLLADOR: Necrovalle
//* VERSION: 0.4a
//* repositorio: https://github.com/Necrovalle/dataLoggerHR_chamber.git 
//****************************************************************************
//Comamdo de compilacion:
//gcc data_logger_CHR.c -o hrDatalogger -pthread `pkg-config --cflags --libs gtk+-2.0`

//------------------------------------------------------------------ LIBRERIAS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>

//----------------------------------------------------- DECLARACIONES GLOBALES

int Cm=0;
unsigned char cmd = 'L';
struct termios tio;
struct termios stdio;
struct termios old_stdio;
int tty_fd;
unsigned char c='D';
gchar *puertoSerie;
gchar *nombreFile;
//Declacion de punteros de la ventana
GtkWidget *ventana, *layout;
//declaracion de entradas
GtkWidget *entPuerto, *entFile;
//Declatacion de botones
GtkWidget *btnConnect, *btnDesc, *btnAdquirir, *btnDetener;

//---------------------------------------------------------- FUNCIONES PROPIAS
//Manejo de hilo
void *thread_routine(void *arg){
	int ST = 2000;
	tcgetattr(STDOUT_FILENO,&old_stdio);
	memset(&stdio,0,sizeof(stdio));
	stdio.c_iflag=0;
	stdio.c_oflag=0;
	stdio.c_cflag=0;
	stdio.c_lflag=0;
	stdio.c_cc[VMIN]=1;
	stdio.c_cc[VTIME]=0;
	tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
	tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // make the reads non-blocking

	memset(&tio,0,sizeof(tio));
	tio.c_iflag=0;
	tio.c_oflag=0;
	tio.c_cflag=CS8|CREAD|CLOCAL; // 8n1, see termios.h for more information
	tio.c_lflag=0;
	tio.c_cc[VMIN]=1;
	tio.c_cc[VTIME]=5;

	//tty_fd=open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);
	tty_fd=open(puertoSerie, O_RDWR | O_NONBLOCK);
	if (tty_fd == -1){
		//mensaje de error
	} else {      
		cfsetospeed(&tio,B9600);            // 9600 baud
		//cfsetispeed(&tio,B9600);            // 9600 baud
		tcsetattr(tty_fd,TCSANOW,&tio);

		while (c!='q'){
			if (read(tty_fd,&c,1)>0) write(STDOUT_FILENO,&c,1);
			// if new data is available on the serial port, print it out
			if (Cm > 1999){
				Cm = 0;
				write(tty_fd, &cmd, 1);
			} else {
				Cm++;
				delay_us(ST);
			}
			read(STDIN_FILENO,&c,1);
			//if (read(STDIN_FILENO,&c,1)>0)  write(tty_fd,&c,1);
			// if new data is available on the console, send it to the serial port
		}
		close(tty_fd);
		tcsetattr(STDOUT_FILENO,TCSANOW,&old_stdio);
		g_print("Adquisiscion detenida... \n");
	}
}

void delay_us(int milli_seconds)
{
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while ((int)clock() < (int)start_time + milli_seconds)
        ;
}

void fnConectar(){
	puertoSerie = gtk_entry_get_text(entPuerto);
	gtk_widget_set_sensitive(btnConnect, FALSE);
	gtk_widget_set_sensitive(btnDesc, TRUE);
	gtk_widget_set_sensitive(btnAdquirir, TRUE);
	g_print("Puero: %s \n", puertoSerie);
}

void fnDesconectar(){
	gtk_widget_set_sensitive(btnConnect, TRUE);
	gtk_widget_set_sensitive(btnDesc, FALSE);
}

void fnAdquirir(){
	c = 'a';
	//Activar el hilo
	pthread_t thread1;
	gtk_widget_set_sensitive(btnDetener, TRUE);
	gtk_widget_set_sensitive(btnDesc, FALSE);
	gtk_widget_set_sensitive(btnAdquirir, FALSE);
	gtk_widget_set_sensitive(entPuerto, FALSE);
	gtk_widget_set_sensitive(entFile, FALSE);
	nombreFile = gtk_entry_get_text(entFile);
	if (nombreFile[0] == '\0'){
		nombreFile = "salida.csv";
	}
	g_print("Nombre del archivo: %s \n", nombreFile);
	int value = 0;		//identificador del hilo
	if (0 != pthread_create(&thread1, NULL, thread_routine, &value))
	//prototipo de funcion para hilo
	{
		//mensaje de fall del hilo
	}
}

void fnDetener(){
	c = 'q';
	gtk_widget_set_sensitive(btnDetener, FALSE);
	gtk_widget_set_sensitive(btnDesc, TRUE);
	gtk_widget_set_sensitive(btnAdquirir, TRUE);
	gtk_widget_set_sensitive(entPuerto, TRUE);
	gtk_widget_set_sensitive(entFile, TRUE);
}


//--------------------------------------------------------------- FUNCION MAIN
int main (int argc, char **argv){
	//Declaracion de etiquetas
	GtkWidget *lblSecConect, *lblPuerto, *lblFile, *lblEtMin, *lblEtAct;
	GtkWidget *lblEtMax, *lblEtTamb, *lblEtHR, *lblEtH1, *lblEtH2, *lblLine1;
	GtkWidget *lblLine2, *lblLine3, *lblLine4, *lblTamb, *lblHR, *lblH1;
	GtkWidget *lblH2, *lblTambMin, *lblHRMin, *lblH1Min, *lblH2Min;
	GtkWidget *lblTambMax, *lblHRMax, *lblH1Max, *lblH2Max;

	//Modificacion de fuente
	PangoFontDescription *df;
	df = pango_font_description_from_string("Monospace");
	pango_font_description_set_size(df,16*PANGO_SCALE);

	//configuracion de la ventana main
	gtk_init(&argc, &argv);
	ventana = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(ventana), GTK_WIN_POS_CENTER);
	g_signal_connect(ventana, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(ventana), "Prueba de humedad comparativa");
	gtk_container_set_border_width(GTK_CONTAINER(ventana), 0);
	//Declarar tamaño de ventana fijo
	gtk_widget_set_size_request(ventana, 620, 240);
	gtk_window_set_resizable(GTK_WINDOW(ventana), FALSE);
	layout = gtk_layout_new(NULL, NULL);
	
	//Creacion de etiquetas
	lblSecConect 	= gtk_label_new("Conexion serie:");
	lblPuerto 		= gtk_label_new("Puerto:");
	lblFile			= gtk_label_new("Nombre del archivo:");
	lblEtMax		= gtk_label_new("Maximo:");
	gtk_widget_modify_font(lblEtMax, df);
	lblEtAct		= gtk_label_new("Actual:");
	gtk_widget_modify_font(lblEtAct, df);
	lblEtMin		= gtk_label_new("Minimo:");
	gtk_widget_modify_font(lblEtMin, df);
	lblEtTamb		= gtk_label_new("T[am]:");
	gtk_widget_modify_font(lblEtTamb, df);
	lblEtHR			= gtk_label_new("%HR:");
	gtk_widget_modify_font(lblEtHR, df);
	lblEtH1		= gtk_label_new("%H1:");
	gtk_widget_modify_font(lblEtH1, df);
	lblEtH2		= gtk_label_new("%H2:");
	gtk_widget_modify_font(lblEtH2, df);
	lblLine1		= gtk_label_new("_______________________________");
	gtk_widget_modify_font(lblLine1, df);
	lblLine2		= gtk_label_new("_______________________________");
	gtk_widget_modify_font(lblLine2, df);
	lblLine3		= gtk_label_new("_______________________________");
	gtk_widget_modify_font(lblLine3, df);
	lblLine4		= gtk_label_new("_______________________________");
	gtk_widget_modify_font(lblLine4, df);
	lblTambMin		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblTambMin, df);
	lblHRMin		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblHRMin, df);
	lblH1Min		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblH1Min, df);
	lblH2Min		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblH2Min, df);
	lblTamb			= gtk_label_new("00.0");
	gtk_widget_modify_font(lblTamb, df);
	lblHR			= gtk_label_new("00.0");
	gtk_widget_modify_font(lblHR, df);
	lblH1			= gtk_label_new("00.0");
	gtk_widget_modify_font(lblH1, df);
	lblH2			= gtk_label_new("00.0");
	gtk_widget_modify_font(lblH2, df);
	lblTambMax		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblTambMax, df);
	lblHRMax		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblHRMax, df);
	lblH1Max		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblH1Max, df);
	lblH2Max		= gtk_label_new("00.0");
	gtk_widget_modify_font(lblH2Max, df);

	//Creacion de entradas
	entPuerto = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entPuerto), 22);
	gtk_entry_set_width_chars(GTK_ENTRY(entPuerto), 22);
	entFile = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entFile), 22);
	gtk_entry_set_width_chars(GTK_ENTRY(entFile), 22);
	
	//Creacion de botones
	btnConnect = gtk_button_new_with_label("Conectar");
	btnDesc = gtk_button_new_with_label("Desconectar");
	btnAdquirir = gtk_button_new_with_label(" Adquirir datos...  ");
	btnDetener = gtk_button_new_with_label("Detener adquisiscion");
	g_signal_connect(btnConnect, "clicked", G_CALLBACK(fnConectar), NULL);
	g_signal_connect(btnDesc, "clicked", G_CALLBACK(fnDesconectar), NULL);
	g_signal_connect(btnAdquirir, "clicked", G_CALLBACK(fnAdquirir), NULL);
	g_signal_connect(btnDetener, "clicked", G_CALLBACK(fnDetener), NULL);
	gtk_widget_set_sensitive(btnDesc, FALSE);
	gtk_widget_set_sensitive(btnAdquirir, FALSE);
	gtk_widget_set_sensitive(btnDetener, FALSE);

	//Posisionamiento
	gtk_layout_put(GTK_LAYOUT(layout), lblSecConect, 10, 10);
	gtk_layout_put(GTK_LAYOUT(layout), lblPuerto, 11, 30);
	gtk_layout_put(GTK_LAYOUT(layout), entPuerto, 10, 50);
	gtk_layout_put(GTK_LAYOUT(layout), btnConnect, 10, 80);
	gtk_layout_put(GTK_LAYOUT(layout), btnDesc, 80, 80);
	gtk_layout_put(GTK_LAYOUT(layout), lblFile, 11, 120);
	gtk_layout_put(GTK_LAYOUT(layout), entFile, 10, 140);
	gtk_layout_put(GTK_LAYOUT(layout), btnAdquirir, 10, 170);
	gtk_layout_put(GTK_LAYOUT(layout), btnDetener, 10, 200);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtMax, 200, 65);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtAct, 200, 120);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtMin, 200, 175);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtTamb, 300, 20);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtHR, 390, 20);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtH1, 460, 20);
	gtk_layout_put(GTK_LAYOUT(layout), lblEtH2, 530, 20);
	gtk_layout_put(GTK_LAYOUT(layout), lblLine1, 200, 25);
	gtk_layout_put(GTK_LAYOUT(layout), lblLine2, 200, 80);
	gtk_layout_put(GTK_LAYOUT(layout), lblLine3, 200, 135);
	gtk_layout_put(GTK_LAYOUT(layout), lblLine4, 200, 190);
	gtk_layout_put(GTK_LAYOUT(layout), lblTambMax, 310, 65);
	gtk_layout_put(GTK_LAYOUT(layout), lblHRMax, 385 ,65);
	gtk_layout_put(GTK_LAYOUT(layout), lblH1Max, 455 ,65);
	gtk_layout_put(GTK_LAYOUT(layout), lblH2Max, 525 ,65);
	gtk_layout_put(GTK_LAYOUT(layout), lblTamb, 310 ,120);
	gtk_layout_put(GTK_LAYOUT(layout), lblHR, 385, 120);
	gtk_layout_put(GTK_LAYOUT(layout), lblH1, 455, 120);
	gtk_layout_put(GTK_LAYOUT(layout), lblH2, 525 ,120);
	gtk_layout_put(GTK_LAYOUT(layout), lblTambMin, 310 ,175);
	gtk_layout_put(GTK_LAYOUT(layout), lblHRMin, 385 ,175);
	gtk_layout_put(GTK_LAYOUT(layout), lblH1Min, 455 ,175);
	gtk_layout_put(GTK_LAYOUT(layout), lblH2Min, 525 ,175);

	gtk_container_add(GTK_CONTAINER(ventana), layout);
	gtk_widget_show_all(ventana);
	gtk_main();
	return 0;

}
//----------------------------------------------------------- FIN DEL PROGRAMA
