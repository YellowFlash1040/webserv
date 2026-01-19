const catContainer = document.getElementById('catContainer');
const uploadForm = document.getElementById('uploadForm');
const uploadMessage = document.getElementById('uploadMessage');

const loadedCats = new Set();

// 1️⃣ Load all existing cats for display
function loadCats() {
	fetch('/uploads/')
		.then(res => res.text())
		.then(html => {
			// catContainer.innerHTML = '';
			const parser = new DOMParser();
			const doc = parser.parseFromString(html, 'text/html');

			const links = doc.querySelectorAll('a');

			links.forEach(link => {
				const filename = link.textContent.trim();
				if (!filename || filename === '.' || filename === '..' || filename === '../' || filename.endsWith('/'))
					return;
				
				// Only add new files
				if (loadedCats.has(filename)) return;
				loadedCats.add(filename);
				
				const div = document.createElement('div');
				div.className = 'cat-item';

				const img = document.createElement('img');
				img.src = '/uploads/' + encodeURIComponent(filename);
				img.alt = filename;

				const btn = document.createElement('button');
				btn.className = 'delete-btn';
				btn.textContent = 'X';

				btn.addEventListener('click', () => {
					fetch('/upload_delete/' + encodeURIComponent(filename), { method: 'DELETE' })
						.then(res => {
							if (res.ok) {
								div.remove();
								loadedCats.delete(filename); // remove from cache
								uploadMessage.textContent = `Deleted ${filename} successfully!`;
								uploadMessage.style.backgroundColor = '#28a745';
								uploadMessage.style.visibility = 'visible';
								setTimeout(() => { uploadMessage.style.visibility = 'hidden'; }, 3000);
							} else {
								console.error('Failed to delete', filename, res.status);
								uploadMessage.textContent = `Failed to delete ${filename}!`;
								uploadMessage.style.backgroundColor = '#dc3545';
								uploadMessage.style.visibility = 'visible';
								setTimeout(() => {
									uploadMessage.style.visibility = 'hidden';
									uploadMessage.style.backgroundColor = '#28a745';
								}, 3000);
							}
						})
						.catch(err => {
							console.error('Delete error', err);
							uploadMessage.textContent = `Delete error for ${filename}!`;
							uploadMessage.style.backgroundColor = '#dc3545';
							uploadMessage.style.visibility = 'visible';
							setTimeout(() => {
								uploadMessage.style.visibility = 'hidden';
								uploadMessage.style.backgroundColor = '#28a745';
							}, 3000);
						});
				});

				div.appendChild(img);
				div.appendChild(btn);
				catContainer.appendChild(div);
			});
		})
		.catch(err => console.error('Load cats error', err));
}

// 2️⃣ Upload new cats
uploadForm.addEventListener('submit', function (e) {
	e.preventDefault();
	const formData = new FormData(uploadForm);
	fetch('/upload_post/', { method: 'POST', body: formData })
		.then(res => {
			if (res.ok) {
				uploadMessage.textContent = 'Upload successful!';
				uploadMessage.style.backgroundColor = '#28a745';
				uploadMessage.style.visibility = 'visible';
				setTimeout(() => { uploadMessage.style.visibility = 'hidden'; }, 3000);

				loadCats();
			} else {
				console.error('Upload failed:', res.status);
				uploadMessage.textContent = 'Upload failed!';
				uploadMessage.style.backgroundColor = '#dc3545';
				uploadMessage.style.visibility = 'visible';
				setTimeout(() => {
					uploadMessage.style.visibility = 'hidden';
					uploadMessage.style.backgroundColor = '#28a745';
				}, 3000);
			}
		})
		.catch(err => {
			console.error('Upload error', err);
			uploadMessage.textContent = 'Upload error!';
			uploadMessage.style.backgroundColor = '#dc3545';
			uploadMessage.style.visibility = 'visible';
			setTimeout(() => {
				uploadMessage.style.visibility = 'hidden';
				uploadMessage.style.backgroundColor = '#28a745';
			}, 3000);
		});
});

// Initial gallery load
loadCats();
