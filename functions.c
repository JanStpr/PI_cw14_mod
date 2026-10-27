//najbardziej elegeancko byłoby stworzyć osobyny plik tree.utils.h (oraz plik .c), w którym będą surowe funckje do obsługi drzewa, oraz inny plik z funkcjami do wyświetlania drzewa oraz obsługi programu w konsoli. Ja jednak zrobię wszystko w funcitions.c, żeby zaoszczędzić trochę czasu i tak nad tym długo siedzę!.

#include <stdio.h>
#include <stdlib.h> //dla polecenia free
#include <stdbool.h> //dla typu bool
#include <string.h> //do strncpy
#include <limits.h> //żeby nie oddawać kluczy o wielkości maksymalnej unsinged int

//=====================================================================
//USTAWIENIA
//=====================================================================

#define MAX_NODE_DISPlAY_LEN 3

//czas czekania w sekundach
#define WRONG_INPUT_WAIT_TIME 1

#define LINES_IN_NODE_DISPLAY 2 //bez problemu można to zmienić, wystarczy tylko dopisać do funckji makearraytree_rec co chcemy, żeby było w tej nowej linii

#define ADD_STARTVALS 1 //czy dodać watości początowe do drzewa?
    #define STARTVALS_NUM 15
    #define STARTVALS_MINKEY 0
    #define STARTVALS_MAXKEY 15
//=====================================================================
//USTAWIENIA
//=====================================================================


//=====================================================================
//UŻYTECZNE FUNKCJE z MOJEJ BIBLIOTEKI
//=====================================================================

//sprawdza, czy wystąpił błąd i jeśli wystąpił, to przerywa program
void errexit(const _Bool condition, const char * restrict const message){
        if(condition){	
                fprintf(stderr, "\nBŁĄD: %s\n", message);
                exit(1);
        }
}

//bezpieczne calokowanie
void * scalloc(const size_t amount, const size_t size_of_one){   
    void * newmem=calloc(amount,size_of_one);
    errexit(!newmem,"MEMORY ALLOCATION FAILED");
    return newmem;
}

//sprawdza, czy wystąpił błąd. Zwraca 1 dla błędu, 0 dla braku błędu
_Bool errnotexit(const _Bool condition, const char * restrict const message){
    if(condition){	
            fprintf(stderr, "\nBŁĄD: %s\n", message);
            return 1;
    }
    return 0;
}

#define iferr_waitreturn(condition,message,waittime,returnval)\
do { if( errnotexit( condition, message ) ) { wait( waittime ); return returnval; } }while(0)

//ten while to sztuczna konsturkcja (i tak to polecenie wyona się tylko raz.) po to, żeby polecenie wewnętrzne było zapakowane w nawias (żeby żaden else się nie przyczepił do tego ifa),  oraz żeby możnabyło wstawić na koniec średnik

//przykładowe użycie:
/* iferr_waitreturn( 
T->root==NULL, 
"\n\nDRZEWO JEST PUSTE", 
<makro przechowujące czas>, ); */ //tutaj trzeba dać puste pole, bo nic nie ma być zwracane

//bezpieczne alokowanie
void * smalloc(const size_t size){   
    void * newmem=malloc(size);
    errexit(!newmem,"MEMORY ALLOCATION FAILED");
    return newmem;
}

//losownik liczb - dodałem to na zaś, bo może będę losował wartości dodawane
#include <time.h> //do losowania liczb na podstawie czasu
#include <stdlib.h> //do rand()
//robię makro a nie funkcję, bo wywoływanie funckji kosztuje dodatkowy czas i pamięć, a makro jest bezkosztowe
#define randint(min,max) ( rand() % ( (max) - (min) +1 ) + (min) )



//słuzy do czyszczenia bufora z stdin. Musi mieć pod ręką zmienną pomocną
#define clearstdin(character) do{ while ((character = getchar()) != '\n' && character != EOF); }while(0)

//wersja z inicjalizają zmiennej pomocniczej (np. na wypadek, jeśli ta funkcja jest wywoływana w funkcji tylko jeden raz)
#define clearstdin_once do{ char clearstdincharactervariable102;\
while ((clearstdincharactervariable102 = getchar()) != '\n' && clearstdincharactervariable102 != EOF); }while(0)

//te do{}while(0) są po to, żeby odizolwoać kod czyściciela od reszty kodu. Dzięki temu, można: bezkarnie wrzucać je do ifów, bezkarnie wywoływać kilka razy pod rząd


//biblioteki do czyszczenia konsoli i czekania
#ifdef __linux__
	#include <unistd.h> //do czyszczenia
    #include <threads.h> //do spania (bo w moim kompilatorze z flagą c17, wszystkie nanosleepy i usleepy są blokowane!!!)
#elif defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

#define DO_CLEARSCREEN 1

void clearscreen(){
    #if DO_CLEARSCREEN==1
        #ifdef __linux__
            system("clear");
        #elif defined(_WIN32) || defined(_WIN64)
            system("cls");
        #endif
    #endif

    return;
}

//ile ma program zaczekać (w sekundach, precyzja: do milisekund (0.001) )
void wait(double t){
    t = t<0?-t:t;   //usuwa ewentualnego minusa
    
    #ifdef __linux__
        struct timespec ts;
        ts.tv_sec = (time_t)t;  //sekundy
        ts.tv_nsec = (long)((t - (double)ts.tv_sec) * 1e9); //nanosekudny
        
        thrd_sleep(&ts, NULL);  //gemini doradził

	#elif defined(_WIN32) || defined(_WIN64)
        unsigned long time = (unsigned long)(t*1000);  //przerabia sekundy na milisekundy
    	Sleep( time );
    #endif
}

//=====================================================================
//UŻYTECZNE FUNKCJE z MOJEJ BIBLIOTEKI
//=====================================================================


//=====================================================================
//FUNKCJE DO OBŁUGI DRZEWA - DLA PROGRAMISTY
//=====================================================================
typedef struct node{
    unsigned int key;
    void *data;
    struct node *parent;
    struct node *left;
    struct node *right;
}node;

typedef struct tree{    //taka struktura się przyda na wypadek, gdybym chciał przechowywać jakieś metadane o drzewie
    struct node *root;
}tree;

//robię to jako osobną funkcję na wypadek, gdybym chciał coś więcej robić przy inicjalizacji drzewa
void inittree(tree  * restrict const T){
    T->root=NULL;
}


void cleartree_rec(node * root){
    if( !root ) return; //jeśli root jest NULL
    cleartree_rec( root->left );
    cleartree_rec( root->right );
    free(root);
}

void cleartree_data_rec(node * root){
    if( !root ) return; //jeśli root jest NULL
    cleartree_data_rec( root->left );
    cleartree_data_rec( root->right );
    free(root->data);
    free(root);
}

//czyści z pamięci strukturę drzewa i (opcjonalnie)dane. UWAGA; każdy ze wskaźników musi koniecznie wskazywać unikalną wartość. Jeśli niektóre węzły będą wskazywać na te same miejsca pamięci, to wtedy ta funkcja wywoła błąd
void cleartree(tree * const T, const bool cleardata){
    //zamiast dwóch funckji rekurencyjnych mógłbym zrobić jedną z warunkiem if, ale ja nie chcę, bo wtedy ten if byłyb niepotrzebnie za każym razem sprawdzany, a węzłów przecież mozę być wiele w drzewie!. Mógłybm też robić makro kóre pyta, czy cleartree ma czyścic dane czy nie, ale mi się tkaie rozwiążnie nie podoba, bo ja może chcę czasem wywołąć z czyszczeniem danych, a raz bez!!
    if(cleardata)   cleartree_data_rec(T->root);
    else    cleartree_rec(T->root);

    T->root = NULL;
}

//szukanie węzła-----------------------------------------------------

//Szuka węzła o danym kluczu w poddrzewie. Funkcja rekurencyjna
//Funkcja zwraca wskaźnik na węzeł o danym kluczu, bądź wskaźnik na węzeł, po którym powinien pojawić się ten, którego szukamy (w sytuacj, kiedy nie ma węzła o szukanym kluczu. Zwracanie takeij wartości będzie bardzo użyteczne dla funkcji addnode, ale sprawia, że zewnętrzy użytkownik raczej nie powinien używać tej funkcji, bo nie zrozumie, że ona nie zwraca NULL przy braku znalezienia.)
//OBługa zwracanej wartości: sprawdzić, czy klucz wskazywanego węzła jest równy szukanemu kluczowi. Jeśli tak, to znaleziono wartość, a jeśli nie, to znaczy, że wskazywany jest węzeł, po którym powinien wylądować nasz węzeł)
node * BST_search_rec(node * const root, unsigned int key){
    //!!!zachowany jest niezmiennik: root nigdy nie jest NULLEM. (przed wywołaniem samej siebie, ta funkcja sprawdza, czy przypadkiem nie próbuje się wywołać dla roota będącego NULLEM)
    unsigned int rootkey = root->key;    //przypisujęto do zmiennej, żebym nie musiałtego wielokrotnie liczyć w warunku
    if( key == rootkey )    return root;
    else if( key > rootkey ){
        if( !root->right )  return root;
        return BST_search_rec(root->right,key);
    }
    else{
        if( !root->left )  return root;
        return BST_search_rec(root->left,key);
    }
}

//wszystkie funkcję pisałem sam, z głowy, dla ćwiczenia. Dlatego to mi tyle zajęło
//szuka w drzewie węzła o danym kluczu
node * BST_searchtree(const tree  * restrict const T, unsigned int key){
    if( T->root==NULL )  return T->root;
    node * found = BST_search_rec( T->root, key );
    if( found->key!=key )   return NULL;
    else    return found;
}

//szukanie węzła-----------------------------------------------------

//dodawanie węzła-----------------------------------------------------

//funkcje z dopiskiem BST_ są funkcjami dla drzew przeszukiwań. funkcje bez tego przedrostka, to funkcje działające dla każdego rodzaju drzewa
//Dodaje węzeł do drzewa BST. Zwraca wskaźnik na nowy węzeł bądź NULL, jeśli dodawnia węzła się nie powiodło (przez to, że istnieje już węzeł o dany kluczu)
node * BST_addnode(tree  * restrict const T, void * data, unsigned int key){
    iferr_waitreturn(key==UINT_MAX,"NIE MOŻNA PRZYPISAĆ KLUCZA O TAKIEJ, LUB WIĘKSZEJ WARTOŚCI",WRONG_INPUT_WAIT_TIME,NULL);

    if( T->root==NULL ){
        node * newnode= (node *)smalloc(sizeof(node));
        newnode->data=data;
        newnode->key=key;

        T->root=newnode;
        newnode->parent = newnode->left = newnode->right = NULL;
        return newnode;
    }
    else{
        node * found = BST_search_rec( T->root, key);   //już był sprawdzony warunek pustości drzewa, więc nie muszę tego drugi raz robić
        if( found->key!=key ){  //jeśli klucza elementu, który chcemy dodać, jeszcze nie ma w drzewie, to trzeba dodać nowy węzeł
            node * newnode= (node *)smalloc(sizeof(node));
            //nadanie odpowiednich wartości newnode'owi
            newnode->data=data;
            newnode->key=key;
            newnode->left = newnode->right = NULL;

            //wiązanie parenta z dzieckiem i vice versa
            newnode->parent = found;
            if( newnode->key < found->key ) found->left = newnode;
            else    found->right = newnode;

            return newnode;
        }
        return NULL;    //jeśli element o tym kluczu już występuje w drzewie. Jeśli drzwo bst miałoby być zbiorem (czyli wartość jest kluczem), to nie dodanie elementu niczego nie zmienia, bo ten element, który dodajemy niczym nie różni się od tego, któy już jest. Ja jednak, na mocy polceniea, robię w istocie swego rodzaju słwonik, a zatem wartości mogą być ówne od klcuczy. Dlatego też całkiem problematyczne jest, jeśli dodawanie się nie powiodło przez powielenie klucza. Najlepiej jest poinformować o tym programistę, żeby obsłużył taki wyjątek (a dla zbioru zrobiłbym incaczej: zwracałbym po prsotu adres istniejącego już węzła)
    }    
}
//dodawanie węzła-----------------------------------------------------

//następnik i poprzednik-----------------------------------------------------

//dla szukania następnika (1) - prawa, dla poprzednika (0) - lewa
node * right_orleft(node * const root, const bool succ){
    return succ?root->right:root->left;
}

//dla szukania następnika (1) - lewa, dla poprzednika (0) - prawa
node * left_orright(node * const root, const bool succ){
    return succ?root->left:root->right;
}

node * BST_neighbour_downstairs(node * const root, const bool succ){
    if( right_orleft(root,succ)!=NULL ){ //sprawdzam ten warunek jeszcze raz, bo ta funkcja może przecież być też wywołwana niezależnie!
        node * ptr = right_orleft(root,succ);
        while( left_orright(ptr,succ)!=NULL )    ptr = left_orright(ptr,succ);
        return ptr;
    }
    else   return NULL;    //jeśli następnik nie jest poniżej roota w drzewie
}

node * BST_neighbour_upstairs(node * const root, const bool succ){
    node * ptr = root;
    node * upperptr = root->parent;
    while( upperptr!=NULL && left_orright(upperptr,succ)!=ptr ){ ptr = upperptr; upperptr=upperptr->parent; }
    return upperptr; //Jeśli w drzewie nie ma następnika danego węzła (czyli jest on największym elementem), to upperptr będzie NULL. Jeśli jest następnik, to upperptr go wskaże
}

//szuka następnika/poprzednika węzła. Root nie może przyjąć wartości NULL!
//succ - succesor - pytanie, czy szukamy następnika (1 - tak, 0 - nie, czyli szykamy poprzednika)
//dla szukania poprzendika, trzeba uruchomić funckjęz succ==0
node * BST_neighbour(node * const root, const bool succ){
    if( right_orleft(root,succ)!=NULL ){    //jeśli następnik jest pod rootem
        return BST_neighbour_downstairs(root, succ);
    }
    else{   //jeśli następnik jest nad rootem
        return BST_neighbour_upstairs(root, succ);
    }
}
//uwaga. Ja dlatego rozbiłem powyższą ufnckję na dwie podfunkcje, ponieważ one będą użyteczne przy (ewentualnym) pisaniu funkcji usuwającej węzeł

//następnik i poprzednik-----------------------------------------------------

//min i max-----------------------------------------------------

node * BST_min(const tree  * restrict const T){
    if( !T->root )   return NULL;    //jeśli drzewo jest puste

    node * ptr = T->root;
    while( ptr->left!=NULL )    ptr = ptr->left;
    return ptr;
}

node * BST_max(const tree  * restrict const T){
    if( !T->root )   return NULL;    //jeśli drzewo jest puste

    node * ptr = T->root;
    while( ptr->right!=NULL )    ptr = ptr->right;
    return ptr;
}
//min i max-----------------------------------------------------

//reprezentacja tablicowa drzewa-----------------------------------------------------


//tutaj powinny być inline'y, ale ja ich nie robię, bo mi to w c17 nie działa!
int left(int n){return 2*n + 1;}
int right(int n){return 2*n + 2;}
int parent(int n){return (n-1)/2;}

typedef struct arraytree{
    node ** arr;
    int len;
}arraytree;


//w tej fukcji działąmy na oryginalmy arrtee oraz oryginalny size. Dlatego każdy z nich dostał jedną dodatkową gwiazdkę
//Zawsze jest zachowany niezmiennik: jeśli poniższa funkcja została wywołana dla roota, to znaczy, że jest na niego miejsce w tablicy. (dowód - indukcyjny: na początku tak jest (tablica z 1 elementem), a potem też jest zachowany, bo zawsze przed wywołaniem funkcji wypełniającej lewego i prawego syna, wześniej sprawdzane jest, czy jest na nich miejsce)
//OPIS: funkcja tworzy reprezentację tablicową dla danego (pod)drzewa. Tablica przechowuje w swoich komórkach wskaźniki na węzły drzewa.
void makearraytree_rec(node * const root, int index, node *** arrtree, int * size){
    if( root ) {    //jeśli węzeł nie jest NULLem (czyli istieje), to go wpisz do listy. A jeśli jest NULLEM, to niczego nie trzeba robić, bo nie ma on dzieci, a on sam jest już wpisany w tablicę, która ma NULL jako wartości domyślne
        node **tarrtree = *arrtree;    //robię raz dereferencję, bo to jest kosztona operacja. tarrtree- True array tree (czyli pierwotny adres tej tablicy)
        tarrtree[index] = root;
        //jeśli w tablicy braknie miejsca, to trzeba ją rozszerzyć (ona zawsze zajmie tyle miejsac, to drzewo pełne o danej wysokości)
        if( right(index)>=*size ){
            int oldsize = *size;
            *size = (*size + 1)*2 - 1;
            *arrtree = (node **)realloc(tarrtree, (*size)*sizeof(node *) );
            tarrtree = *arrtree; //wskaźnik trzeba uaktualnić
            for( int i=oldsize; i<*size; i++ )   (tarrtree)[i]=NULL; //zeruje nowo zaalokowane obszary. Przez to, wartość NULL mają wszystkie nieistniejące w pierwotnym drzewie węzły.
        }
        makearraytree_rec( root->left, left(index), arrtree, size );    //obsługa lewego syna
        makearraytree_rec( root->right, right(index), arrtree, size );    //obsługa prawego syna
    }
}

//PAMIĘTAĆ O TYM, żeby potem po zakończeniu używania arraytree, zwalniać pamięć A.arrtree (bo makearraytree alokuje pamięć na stosie)
void makearraytree(const tree  * const T, arraytree  * restrict const A){
    if( !T->root ){   
        A->len=0;
        A->arr=NULL;
        return; //jeśli drzewo jest puste
    }    

    int treesize = 1;
    node ** arrtree = (node **)smalloc(treesize*sizeof(node *));
    arrtree[0]=NULL;    //tablica w nieruszonych miejsacch ma mieć NULL, żeby potem było wiadomo gdzie drzewa ma węzły, a gdzie nie
    makearraytree_rec( T->root, 0, &arrtree, &treesize);

    A->arr = arrtree;
    A->len = treesize;
}

//reprezentacja tablicowa drzewa-----------------------------------------------------

//równoważenie drzewa----------------------------------------------------------------
//używam metody 1, ponieważ mój arraytree, idalnie do tego się nada. Poza tym jest to okazja do poćwiczenia qsorta

int compare(const void * a, const void * b){
    unsigned int int_a = ( *((node **) a) )->key;
    unsigned int int_b = ( *((node **) b) )->key;
    return ( int_a>int_b ) - ( int_a < int_b );
}

//ta funkcja jest wywołtwana w BST_balance, gdzie jest zapewnione, że przekazana tablica jest conajmniej trzyelementowa i bez dziur (NULL)
void BST_balance_rec(const arraytree * const restrict sourceA,  tree * destT, int l, int r){
    if( l>r ) return;   //jeśli przedział jest pusty
    int piv = l + (r-l)/2;
    node * currnode = sourceA->arr[piv];    //robię po to, żeby nie musiec dwa razy robić takiego złożoneg ociągu (kosztownych) dereferencji
    BST_addnode(destT, currnode->data, currnode->key);  
    
    BST_balance_rec(sourceA, destT, l, piv-1);
    BST_balance_rec(sourceA, destT, piv+1, r);
}

void BST_balance(tree  * restrict T){ //złożóność obliczeniowa: O(nlog n) (sortowanie)
    if( !T->root )   return;    //jeśli drzewo jest puste

    arraytree A;
    makearraytree(T,&A);

    //mógłbym po prostu wkleić implementajce lsity z mojej bilitoeki, do niej dodawać wszystkie węzły drzewa i potem listę zapisać do dynamicznej talbicy (i juz potem listę usuną, bo się więcej nie przyda). W ten sposób byłoby prościej, ale ja już nie hcciałrem dodawać tej ogromnej implementacji moich list (używałem ich też we wześniejszych projektach), adlatego zdecydowałem się użyć mojego araytree, ale muszę użyć pewnej szutczki, żeby ,,pozbyć" się nustych elementów mojego arraytree.
    node fakenode;
    fakenode.key= UINT_MAX; //można to też uzywskać przez kotnrolowane przepełnenie (... = -1, bo to unsigned)
    //key jest typu unsigned, więc UINT_MAX da największąmożłiwą wartość . Zatem key tego node'a będzie największy - wszytkie nieistniejące node'y zostaną przesunięte do tyłu tablicy, po sortowaniu
    for( int i=0; i<A.len; i++) if( !A.arr[i] ) A.arr[i]=&fakenode;

    qsort(A.arr, A.len, sizeof(node*), compare);    //po porsortowaniu, wszystkie puste klucze (przypięte do fakenode'a) będą na końcu tablicy. W ten sposó mogę je łatwo zignorować w dalszych rachubach.

    int endboundary = A.len-1;  //tą zmienną będę wskazywac na koniec faktycznych danych (ostatni element talbicy, któy jest wskaźnikiem na prawdziwego node'a, a nie fakenode'a)
    while( (A.arr[endboundary])->key == UINT_MAX )  endboundary--;    //wiem na pewno, że drzewo nie jest puste, więc na pewno ma conajmniej jeden, prawdziwy element. Zatem ta pętla na pewno nie przekroczy zakresu, bo startbounadry się zatzryma gdzies wewnątrz talbicy na tym elemencie (albo jakimśinnym prawdizwym)
    //nie da się nijak zrównoważyć drzew 1 i dwuelementowych, więc najlepiej przerwać taką bezsensowną operację
    if( endboundary>1 ){  //jeśli drzewo ma conajmniej trzy węzły
        tree * newT = (tree *)smalloc(1*sizeof(tree));  //drzewo neeT trzeba alokować na stosie, bo ono potem ma zostać w pamięći , po wyjściu z tej funkcji(a gyby nie było robione na stercie, to ono by zniknęło po zakończeniu tej funkcji (automatyczne rozładowywanie pamieci)). Nie chcę uniknąć tej alokacji przez nie robienie smalloca i przypisanie do T->root newT.root (a to by zadziaało poprawnie!), ponieważ drzewo t, poza rootem, może w przyszłośi posiadać jakieśmetadane, zależne od dodawanyche lementów (np. rozmiar). Żeby nowe metadane były porpwane, to wolę tworzyć cąłe nowe drzewo.
        inittree(newT);
        BST_balance_rec(&A, newT, 0, endboundary); //newT staje się zbalansowaną wersją drzewa
        cleartree(T, 0);    //czyści strukturę starego drzewa (ale nie dane !!)
        *T = *newT;
        free(newT);
    }

    free(A.arr);
} 

//równoważenie drzewa----------------------------------------------------------------

//=====================================================================
//FUNKCJE DO OBŁUGI DRZEWA - DLA PROGRAMISTY
//=====================================================================

//=====================================================================
//FUNKCJE DO OBŁUGI DRZEWA - DLA UŻYTKOWNIKA
//=====================================================================

//przerobiłem to na makro, ponieważ nie chcęniepotrzebnej redundacji kodu. Może być to ryzykowne, ale ja wiem co robię
#define user_readnewvariable(variable_type,variable_name,variable_type_flag,text) variable_type variable_name;\
    printf(text);\
    scanf(variable_type_flag,&variable_name);



void readstring(char * text){
    fgets(text, (MAX_NODE_DISPlAY_LEN+1)*sizeof(char) ,stdin);
    bool stdin_needscleaning=1;
    for( int i=0; i<MAX_NODE_DISPlAY_LEN; i++ ){
        if( text[i]=='\n' ){
            text[i]='\0';   //usuwa znak nowej linii, jeśl itakowy jest w stringu (może sięon pojawić jeśli ktoś wpisze nazwę krótszą niż 3)
            stdin_needscleaning=0;  //jeśli gdzieś w środku stringa występuje \n, to znaczy, że fgets wczytał wszystko z stdin, zatem nie trzeba go czyścić (czyszczenie go funckjąclearstrfin wtedy byłoby błędne, bo ona by wtedy czekała, aż użytkownik wproadzi kolejny znak końca linii)
        }
    }

    if( stdin_needscleaning )   clearstdin_once;
}

void u_BST_addnode(tree  * restrict const T){
    char c; //do czyszenia strumienia wejścia
    user_readnewvariable(unsigned int, key,"%u","Podaj klucz(liczbę maks. 3 cfyrową): ");   //jeśli użytkownik poda inną niż całkowita, to wyskoczy błąd - więc będzie komunikat
    clearstdin(c);

    char * text = (char *)smalloc( (MAX_NODE_DISPlAY_LEN+1)*sizeof(char) ); //+1, bo musi być miejsce na /0

    printf("Podaj wartość(trzyliterowe słowo): ");
    readstring(text);

    //dodaje element do listy i zwalnia pamięć, jeśli taki element juz istnieje
    if( BST_addnode(T, text, key)==NULL ){
        free(text);
        iferr_waitreturn( 1 ,"ISTNIEJE JUŻ WĘZEŁ O DANYM KLUCZU",WRONG_INPUT_WAIT_TIME, );
    }
}

void u_BST_searchtree(const tree  * restrict const T){
    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );

    user_readnewvariable(unsigned int, key,"%u","Podaj klucz(liczbę maks. 3 cfyrową):");
    clearstdin_once;

    node * found = BST_searchtree(T,key);

    iferr_waitreturn(found==NULL,"NIE ZNALEZIONO SZUKANEGO ELEMENTU",WRONG_INPUT_WAIT_TIME, );

    printf("\n\n%s",(char *)found->data);
}

void u_BST_searchmin(const tree  * restrict const T){
    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );

    printf("\n\n%s",(char *)BST_min(T)->data);
}

void u_BST_searchmax(const tree  * restrict const T){
    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );
    
    printf("\n\n%s",(char *)BST_max(T)->data);
}

void u_BST_search_succesor(const tree  * restrict const T){
    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );

    user_readnewvariable(unsigned int, key,"%u","Podaj klucz(liczbę maks. 3 cfyrową):");
    clearstdin_once;

    node * found = BST_searchtree(T,key);
    iferr_waitreturn(found==NULL,"NIE ZNALEZIONO ELEMENTU, KTÓREGO NASTĘPNIKA SZUKAMY",WRONG_INPUT_WAIT_TIME, );
    node * succesor = BST_neighbour(found, 1);
    iferr_waitreturn(succesor==NULL,"WSKAZANY ELEMENT JEST NAJWIĘKSZY, ZATEM NIE MA NASTĘPNIKA",WRONG_INPUT_WAIT_TIME, );

    printf("\n\n%s",(char *)succesor->data);
}

void u_BST_search_predecessor(const tree  * restrict const T){
    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );

    user_readnewvariable(unsigned int, key,"%u","Podaj klucz(liczbę maks. 3 cfyrową):");
    clearstdin_once;

    node * found = BST_searchtree(T,key);
    iferr_waitreturn(found==NULL,"NIE ZNALEZIONO ELEMENTU, KTÓREGO NASTĘPNIKA SZUKAMY",WRONG_INPUT_WAIT_TIME, );
    node * predecessor = BST_neighbour(found, 0);
    iferr_waitreturn(predecessor==NULL,"WSKAZANY ELEMENT JEST NAJMNIEJSZY, ZATEM NIE MA POPRZEDNIKA",WRONG_INPUT_WAIT_TIME, );

    printf("\n\n%s",(char *)predecessor->data);
}

void u_search_parent(const tree  * restrict const T){
    #ifdef ISBLACK
        printf("Brak ojca");    return ;
    #endif  

    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );

    user_readnewvariable(unsigned int, key,"%u","Podaj klucz(liczbę maks. 3 cfyrową):");
    clearstdin_once;

    node * found = BST_searchtree(T,key);
    iferr_waitreturn(found==NULL,"NIE ZNALEZIONO ELEMENTU, KTÓREGO OJCA SZUKAMY",WRONG_INPUT_WAIT_TIME, );

    node * par = found->parent;
    if( !par )   printf("\n\nWskazany element to korzeń drzewa - nie ma ojca");
    else    printf("\n\n%s",(char *)found->parent->data);
}

void u_BST_balance(tree  * restrict T){
    iferr_waitreturn(T->root==NULL,"DRZEWO JEST PUSTE",WRONG_INPUT_WAIT_TIME, );
    BST_balance(T);
}
    
//ja będę do każdej funkcji, bez wyjątku, przekazywał wskaźnik na tree, a nie samo tree, ponieważ nie chcę za każdym razem kopiowaćdo funkcji metadanych (wprawdzie jest terazj tylko jedna metadadana, ale może być ich więcej w przyszłości)

//=====================================================================
//FUNKCJE DO OBŁUGI DRZEWA - DLA UŻYTKOWNIKA
//=====================================================================


//=====================================================================
//WYŚWIETLANIE DRZEWA
//=====================================================================

//w zasadzie nodeline jest niepotrzebny, ale gdyby data było bardziej rozbudowanie (np. tablica stringów), to wtedy ten nodeline były potrzebny do wskazywania który wiersz z tejże talbicy cheilibyśmy wypritnować. UWAGA: pierwszmy idneksem będzie 1, bo 0 jest zarezerwoane dla indeksu!
char * datatostring(const node * restrict const root/*, const int nodeline*/){
    char * datastring = (char *)scalloc( MAX_NODE_DISPlAY_LEN+1 , sizeof(char) );   //scalloc od razu zeruje, więc na każym meijscu jest '/0'
    strncpy(datastring, (char *)root->data, MAX_NODE_DISPlAY_LEN);
    //pętla obsługuje napisy krótsze niż zadany limit (w odpowiednie miejsca wsadza spacje)
    for( int i=MAX_NODE_DISPlAY_LEN-1; i>0 && datastring[i-1]=='\0'; i++ )    datastring[i]=' ';

    return datastring;
}

//UWAGA: zarówno datatostirng jak i keytostirng zwracają wskaźńik na obszar pamieci przechowująćy dany napis. Zatem trzeba pamiętac o późniejszym zwolnienu tej pamięci!!
char * keytostring(const node * restrict const root){
    int key = root->key;
    char * keystring= smalloc( (MAX_NODE_DISPlAY_LEN+1) * sizeof(char) );
    //Pętla zczytuje klucz, od końca. Jeśli jakiś krnąbrny użytkownik progrmau nie zastosuje się do mojego poleceniea, żeby klucze były conajwyżej trzyliterowe, to ów twardogłowy zdziwi się widząc, że w wyświetlonym drzewie sąjedynie trzy peirwsze cyfry jego gigantycznych liczb.
    for( int i=MAX_NODE_DISPlAY_LEN-1; i>-1; i-- ){
        keystring[i]= key%10 + '0';
        key/=10;
    }
    keystring[MAX_NODE_DISPlAY_LEN]='\0';
    return keystring;
}

//wypisanie drzewa na konsoli i tak miałoby sens tylko dla małych drzew (większe przecież nie zmieści się na konsoli), dlatego najłatwiej będzie przerobić drzewo z posaci węzłowej na postać tablicową. To bardzo wiele ułatwi. Gdybym chciał np. rekurecnją wyświetlać drzewo, to by się nie udało w takiej psotaci jak ja chcę (korzeń na góze, liśćie na dole), ponieważ rekurencja nie chodzi poziomami, ale inaczej. W nowej reprezanrtji tabliceowej, wszystko będzie proste i oczywiste.

//UWAGA: szczegóły, skąd wziąłem ten sposób wypisywania drzewa, są na kartce w moim zeszycie z PI.
bool printtree(const tree  * restrict const T){
    if( T->root==NULL )  return 0;
    arraytree A;  //A - jak array, cyzli tablica. Z kolei T jest od Tree, czyli drzewo
    makearraytree(T, &A);
    int len = A.len + 1; //zapamiętuje sobie wartość dla wygody (i żeby niepotrzenibe nie mnżyć koszotwynch dereferencji), ale niech nikt mi tu nie marudzi, że w takim razie bez sensu robiłem strkturę arraytree, bo obiłem to patrząc przyszłosćiwoo.

    char * line_to_cut = (char *)smalloc( (len+1) * sizeof(char) );    //to linia białych znaków, która będzie separować węzły na konsoli. My sobie będziemy po portu z tego stringa ucinać taki fragemnt, jaki nam będziew danej chwili potrzebny.
    for( int i=0; i<len; i++ )    line_to_cut[i]=' ';
    line_to_cut[len]='\0';  //znak kończący strigna

    char * empty_node = (char *)smalloc( (MAX_NODE_DISPlAY_LEN+1)*sizeof(char) );  //to string który będzie wklejany w miejsac bez node'ów. Jest to po prostu pasek białych nzaków
    for( int i=0; i<MAX_NODE_DISPlAY_LEN; i++ )    empty_node[i]=' ';
    empty_node[MAX_NODE_DISPlAY_LEN]='\0';  //znak kończący strigna

    int currindex=0;
    int counter = 1;
    int curr_betweenspace = len-1;

    //możliwe sądwa podejściac: 1. budować ogromnego stringa i raz go wyprintować (trzebaby zaalokować dużo pamieci na kopię drzewa w postaci stringa, a poza tym, to będdzie to bardziej złożóne). 2 - printować po koleji wszytko (prostrze, ale dużo operacji IO, zatem trochę wolniejsze)


    int nodeline=0; //robię to przed pętą, żeby nie trzeba było za każym razem tego inicjalizwoać
    int i;
    char * stringbuf;

    while( counter<len ){ // counter musi być <len, ponieważ musi dojść do len/2, czyli ilości liści.
        for( nodeline=0; nodeline<LINES_IN_NODE_DISPLAY; nodeline++ ){
            for( i=0; i<counter; i++ ){//ta pętla zapewnia, że wyprintujemy wszystkie linie, jakie chcemy.
                if( A.arr[ currindex + i ]!=NULL ){    printf("%.*s%.*s%.*s",\
                    curr_betweenspace, line_to_cut,\
                    MAX_NODE_DISPlAY_LEN,    stringbuf= ( nodeline==0?keytostring(A.arr[currindex + i]):datatostring(A.arr[currindex + i]/*,nodeline*/) ) ,\
                    curr_betweenspace-1, line_to_cut); //trzeba +1 do MAX_NODE_..., ponieważ ja alokowałem miejsce na te znaki + znak kończący
                    //UWAGA: jeśli chciałoby się dodać więcej linii do każdego node'a, to trzeba tej instruckji warunkowej do obługi indeksu/danych nie trzeba zmieniać. Bo jako pierwsyz zawsze się wyświetla indeks (dla nodeline==0), a dla kolejnyhc ,to wyśiwetlamy kolejne linie z data
                    //z porady od gemini dowiedziałem się, że można wypisać tylko część stringa, dzięki %.*s (i potem trzeba dać długosć, tekst)
                    free(stringbuf);    //bufor na stringi dostaje przez keotostring i datatstirng nowozaalokowane obszary pamięci (musi tak być, żeby ta warotśc mogła być zwracana. Gdyby te wartości były przechowywane w zaalokowanej talibcy znaków, to ta tablica by znikła po skonczeniu funkcji i w efekcie zwrócony by został wsakźnik na śmeici w pamięci!)
                }
                else    printf("%.*s%s%.*s",\
                    curr_betweenspace, line_to_cut,\
                    empty_node,\
                    curr_betweenspace-1, line_to_cut);
            }
            printf("\n");
        }
        currindex += counter;  //currindex kontroluje od indeksu którego indeksu teraz zaczynam wstawianie. Dlatego jest to ilość wsyzstkich dotąd wpisanych node'ów
        counter *= 2; //mgę zrobić ==, a nie >=, bo wiem, że i skacze co jeden. Counter kontroluje ile mam klocków do wypisania na danym poziomie
        curr_betweenspace = (len/counter)-1;    //przechowuje informacje o długości odstępu lewego (a prawy jest ten sam, ale mniejszy o 1)
        printf("\n");
    }
    free(A.arr);

    free(empty_node);
    free(line_to_cut);
    return 1;
}

//=====================================================================
//WYŚWIETLANIE DRZEWA
//=====================================================================


//=====================================================================
//UI
//=====================================================================

#define BIGLINE "===================================================================="
#define SMALLINE "--------------------------------------------------------------------"

void printui(){
    //tą sztuczkę z cudzyłowami zapoponował gemini
    printf("\
"BIGLINE"\n\
Wybierz operację:\n\n\
0. Zatrzymaj program\n\
1. Dodaj węzeł do BST\n\
2. Znajdź węzeł o danym kluczu\n\
3. Znajdź najmniejszą wartość\n\
4. Znajdź największą wartość\n\
5. Znajdź następnika węzła o danym kluczu\n\
6. Znajdź poprzednika węzła o danym kluczu\n\
7. Znajdź rodzica węzła o danym kluczu\n\
8. Równoważ drzewo\n\
...\n\
"BIGLINE"\n\n");
}
//=====================================================================
//UI
//=====================================================================

//=====================================================================
//GŁÓWNA FUNKCJA PROGRAMU
//=====================================================================

void add_startvals(tree  * restrict const T){   //jeśli chemy dodać jakieś wartości domyślne, to robi to ta funkcja
    char * text;
    unsigned int key;
    if(ADD_STARTVALS){
        for( int i=0; i<STARTVALS_NUM ; i++ ){
            text = (char *)smalloc( (MAX_NODE_DISPlAY_LEN+1)*sizeof(char) );
            for( int j=0; j<MAX_NODE_DISPlAY_LEN; j++ )  text[j]= (char)randint('a','z');
            text[MAX_NODE_DISPlAY_LEN]='\0';

            do{ key = (unsigned int)randint( STARTVALS_MINKEY,STARTVALS_MAXKEY );
            }while(BST_searchtree(T, key));    //generuj klucze, dopóki nie wygenerujesz unikalnego

            BST_addnode(T, text, key );
        }
    }
}

void bst_showcase(){
    tree T;
    inittree(&T);

    int action; //nie bawię się tu w enumy, bo dla mnie czytelniej będzie robić to liczbami (bo użytkownik i tak wybiera działanie liczbą)
    char buf;   //bufor do czyszczenia strumiania wejścia
    add_startvals(&T);
    clearscreen(); 

    while( 1 ){
        if( printtree(&T)) printf("\033[3A\033[J"); //znaki sterująće z ascii. Jeśli printtree się wykonał (czyli drzewo nie jest puste), to utnij mi trzy wiersze (one powstają przez nakłądnie się \n z różnych poziomów, moje roziwąznie teog problemu jest njaprostsze)
        printui();
        scanf("%d",&action);
        clearstdin(buf);    //bufor czyści wejście, na wypadek, gdyby ktoś wrzucił coś niewłaściwego

        if( !action ) break;
        switch(action){
            case 1: u_BST_addnode(&T);    break;
            case 2: u_BST_searchtree(&T);    break;
            case 3: u_BST_searchmin(&T);    break;
            case 4: u_BST_searchmax(&T);    break;
            case 5: u_BST_search_succesor(&T);    break;
            case 6: u_BST_search_predecessor(&T);    break;
            case 7: u_search_parent(&T);    break;
            case 8: u_BST_balance(&T);    break;
            default:   printf("Nieprawidłowe polecenie!"); wait(WRONG_INPUT_WAIT_TIME);   break;
        }
        clearscreen(); 
    }

    cleartree(&T, 1);    //zwalnia pamięć drzewa
}
//=====================================================================
//GŁÓWNA FUNKCJA PROGRAMU
//=====================================================================