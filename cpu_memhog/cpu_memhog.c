int main() {
	int i = 0;
	while(1) {
		volatile int arr[1000];
		for(i = 0 ; i < 1000 ; i++)
			arr[i] = 0;

	}
}
