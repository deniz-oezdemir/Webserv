const JOKE_URL = "https://icanhazdadjoke.com/";

// replace joke
const joke = document.getElementById("joke-target");

async function replaceJoke() {
	try {
		const response = await fetch(JOKE_URL, {
			method: 'GET',
			headers: {
				'Accept': 'application/json',
				'User-Agent': 'My Library (https://github.com/username/repo)' // Replace with your actual user agent
			}
		});
		const processedResponse = await response.json();
		const jokeText = processedResponse.joke;

		// Create or update the joke text
		let jokeElement = joke.querySelector('p');
		if (!jokeElement) {
			jokeElement = document.createElement("p");
			joke.appendChild(jokeElement);
		}
		jokeElement.textContent = jokeText;
	} catch (error) {
		console.error("Error fetching joke:", error);
	}
}

document.getElementById("joke-btn").addEventListener("click", replaceJoke);

