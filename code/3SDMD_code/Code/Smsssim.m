function overall_mssim = Smsssim(img1, num, map, K, win, level, weight, method)

% Multi-scale Structural Similarity Index (MS-SSIM)
% Z. Wang, E. P. Simoncelli and A. C. Bovik, "Multi-scale structural similarity
% for image quality assessment," Invited Paper, IEEE Asilomar Conference on
% Signals, Systems and Computers, Nov. 2003

img1 = imread(img1);
%img2 = imread(img2);
dimension = numel(size(img1));
if dimension==3 img1 = rgb2gray(img1);end


x=double(imread('imShow/R.pgm'));
y=double(imread('imShow/G.pgm'));
z=double(imread('imShow/B.pgm'));

r = uint8(min(max(0, round(x                      + 1.402  * (z - 128))), 255));
g = uint8(min(max(0, round(x - 0.3441 * (y - 128) - 0.7141 * (z - 128))), 255));
b = uint8(min(max(0, round(x + 1.772  * (y - 128)                     )), 255));

combined_RGB = cat(3,r,g,b); 

OutString=num2str(num);
imwrite(combined_RGB, OutString, 'png');

img2 = rgb2gray(combined_RGB);

map = im2double(imread(map));
map = map/2.0 + 0.1;

if (nargin < 2 || nargin > 8)
   overall_mssim = -Inf;
   return;
end

if (~exist('K'))
   K = [0.01 0.03];
end

if (~exist('win'))
   win = fspecial('gaussian', 11, 1.5);
end

if (~exist('level'))
   level = 5;
end

if (~exist('weight'))
   weight = [0.0448 0.2856 0.3001 0.2363 0.1333];
end

if (~exist('method'))
   method = 'product';
end

if (size(img1) ~= size(img2))
   overall_mssim = -Inf;
   return;
end

[M N] = size(img1);
if ((M < 11) || (N < 11))
   overall_mssim = -Inf;
   return
end

if (length(K) ~= 2)
   overall_mssim = -Inf;
   return;
end

if (K(1) < 0 || K(2) < 0)
   overall_mssim = -Inf;
   return;
end
  
[H W] = size(win);

if ((H*W)<4 || (H>M) || (W>N))
   overall_mssim = -Inf;
   return;
end
   
if (level < 1)
   overall_mssim = -Inf;
   return
end


min_img_width = min(M, N)/(2^(level-1));
max_win_width = max(H, W);
if (min_img_width < max_win_width)
   overall_mssim = -Inf;
   return;
end

if (length(weight) ~= level || sum(weight) == 0)
   overall_mssim = -Inf;
   return;
end

if (method ~= 'wtd_sum' & method ~= 'product')
   overall_mssim = -Inf;
   return;
end

downsample_filter = ones(2)./4;
im1 = double(img1);
im2 = double(img2);
for l = 1:level
   [mssim_array(l) mcs_array(l)] = Sssim(im1, im2, map, l, K, win);
%   [M N] = size(im1);
   filtered_im1 = imfilter(im1, downsample_filter, 'symmetric', 'same');
   filtered_im2 = imfilter(im2, downsample_filter, 'symmetric', 'same');
   filtered_map = imfilter(map, downsample_filter, 'symmetric', 'same');
   
   clear im1;
   clear im2;
   clear map;
   im1 = filtered_im1(1:2:end, 1:2:end);
   im2 = filtered_im2(1:2:end, 1:2:end);
   map= filtered_map(1:2:end, 1:2:end);
end

if (method == 'product')
%   overall_mssim = prod(mssim_array.^weight);
   overall_mssim = prod(mcs_array(1:level-1).^weight(1:level-1))*(mssim_array(level).^weight(level));
else
   weight = weight./sum(weight);
   overall_mssim = sum(mcs_array(1:level-1).*weight(1:level-1)) + mssim_array(level).*weight(level);
end
fid = fopen('../outputFile/skeleton_msssim2.txt','a');
%fid = fopen('../outputFile/JPGmsssim.txt','a');
fprintf(fid,'%f\n',overall_mssim);
fclose(fid);