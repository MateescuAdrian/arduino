const CACHE_NAME = 'robot-remote-v3';
const ASSETS = [
    './',
    './index.html',
    './manifest.json',
    './icon.png'
];

self.addEventListener('install', event => {
    // Force the waiting service worker to become the active service worker
    self.skipWaiting();
    event.waitUntil(
        caches.open(CACHE_NAME).then(cache => {
            return cache.addAll(ASSETS);
        })
    );
});

self.addEventListener('activate', event => {
    // Claim any clients immediately
    event.waitUntil(clients.claim());
    event.waitUntil(
        caches.keys().then(keys => {
            return Promise.all(keys.map(key => {
                if (key !== CACHE_NAME) return caches.delete(key);
            }));
        })
    );
});

self.addEventListener('fetch', event => {
    // Only intercept local UI assets, not the ESP32 /drive requests
    if (event.request.url.includes('/drive')) {
        return;
    }
    
    // For local assets, ignore query strings (like ?ts=123) when matching the cache
    event.respondWith(
        caches.match(event.request, { ignoreSearch: true }).then(response => {
            // Because of our auto-refresh script, we actually want to fetch from network if possible, 
            // and fallback to cache if offline. This allows PWA install validation to pass.
            return fetch(event.request).catch(() => response);
        })
    );
});
