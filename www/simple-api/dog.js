const DOG_URL = "https://dog.ceo/api/breeds/image/random";

/*
// append doggos
const doggos = document.getElementById("dog-target");

async function addNewDoggo() {
	const promise = await fetch(DOG_URL);
	const processedResponse = await promise.json();
	const img = document.createElement("img");
	img.src = processedResponse.message;
	img.alt = "Cute doggo";
	doggos.appendChild(img);
}

document.getElementById("dog-btn").addEventListener("click", addNewDoggo);
*/

// replace dog
const doggo = document.getElementById("dog-target");

async function replaceDoggo() {
	const promise = await fetch(DOG_URL);
	const processedResponse = await promise.json();
	// Create or update the image
	let img = doggo.querySelector('img');
	if (!img) {
		img = document.createElement("img");
		doggo.appendChild(img);
	}
	img.src = processedResponse.message;
	img.alt = "Cute doggo";
}

document.getElementById("dog-btn").addEventListener("click", replaceDoggo);
