// Samy Mannane, Donovan Neveux

//Sample of code about a Bounding volume hierarchy
//It is a block of code and not a complete class

// [...]

BVH::BVH(BoundingBox bbox, std::deque<Triangle> triangles) {
			box.first = bbox;
			box.second = triangles;

			this->left = NULL;
			this->right = NULL;

		}

		BVH::BVH(std::deque<Triangle> triangles) {

			box.second = triangles;

			// Updating the bounding box
			for (unsigned int i = 0; i < box.second.size() - 1; i++)
			{
				box.first.update(box.second[i]);
			}

			this->left = NULL;
			this->right = NULL;

		}


		//function to compare triangles on X axis in order to sort them
		static bool  compareX(Triangle t1, Triangle t2) {
			return (t1.center()[0] < t2.center()[0]);
		}


		std::deque<Triangle> triX(std::deque<Triangle> &tris) {
			std::sort(tris.begin(), tris.end(), compareX);
			return tris;
		}


		//function to compare triangles on Y axis in order to sort them
		static bool  compareY(Triangle t1, Triangle t2) {
			return (t1.center()[1] < t2.center()[1]);
		}


		std::deque<Triangle> triY(std::deque<Triangle> &tris) {
			std::sort(tris.begin(), tris.end(), compareY);
			return tris;
		}


		//function to compare triangles on Z axis in order to sort them
		static bool  compareZ(Triangle t1, Triangle t2) {
			return (t1.center()[2] < t2.center()[2]);
		}


		std::deque<Triangle> triZ(std::deque<Triangle> &tris) {
			std::sort(tris.begin(), tris.end(), compareZ);
			return tris;
		}



		//Function to select the axis where we will sort triangles
		std::deque<Triangle> chooseTri(std::deque<Triangle> &tris, int k) {

			std::deque<Triangle> tri;
			switch (k % 3) {
			case 0: tri = triX(tris);
				break;
			case 1: tri = triY(tris);
				break;
			case 2: tri = triZ(tris);
				break;
			}
			return tri;
		}


		bool isLeaf(BVH* bvh) {
			bool leaf = false;
			if (bvh->box.second.size() <= 16) {
				leaf = true;
			}
			return leaf;
		}




		//Partitionning space
		void partition(int k) {

			//Selection of the axis where we will sort and compare triangles
			std::deque<Triangle> tri = chooseTri(box.second, k);
			std::deque<Triangle> tri_left;
			std::deque<Triangle> tri_right;

			if (tri.size() >= 16) {
				//Index of the middle triangle on the selected axis
				int indiceCut = tri.size() / 2;


				for (unsigned int i = 0; i < tri.size(); i++) {

					if (i <= indiceCut) {
						//-----------
						//Add current triangle in the left son of the current node
						tri_left.push_back(tri[i]);
					}
					else {
						//-----------
						//Add current triangle in the right son of the current node
						tri_right.push_back(tri[i]);
					}
				}

				// Initialize deques for left and right sons
				left = new BVH(tri_left);
				right = new BVH(tri_right);

				for (unsigned int cpt = 0; cpt < tri_left.size(); ++cpt)
				{
					left->box.first.update(tri_left[cpt]);
				}

				for (unsigned int cpt = 0; cpt < tri_right.size(); ++cpt)
				{
					right->box.first.update(tri_right[cpt]);
				}

				tri.clear();
				tri_left.clear();
				tri_right.clear();

				//Recursion
				left->partition(k + 1); //call on the left son
				right->partition(k + 1); //call on the right son
			}

		}



		//Let us getting the intersection between the ray and the 'leaf' bounding box of the tree
		RayTriangleIntersection traversingBVH(Ray const & ray, double t1, double t2)
		{
			RayTriangleIntersection i_found;
			RayTriangleIntersection i_found2;

			CastedRay cRay(ray);

			double t_entry;
			double t_exit;
			bool isIntersected = box.first.intersect(ray, t1, t2, t_entry, t_exit);


			// CASE LEAF
			if (box.second.size() <= 16) {

				// Compute closest intersection with the primitives associated with the node
				for (auto it = box.second.begin(), end = box.second.end(); it != end; ++it)
				{
					cRay.intersect(&(*it));
				}
				i_found = cRay.intersectionFound();

			}
			// CASE NOT LEAF : check for sons
			else
			{
				// Compute ray intersection with the sons bounding boxes
				double t_entryL, t_exitL, t_entryR, t_exitR;
				bool isIntersectedLeft = false;
				bool isIntersectedRight = false;
				isIntersectedLeft = left->box.first.intersect(ray, t_entry, t_exit, t_entryL, t_exitL);
				isIntersectedRight = right->box.first.intersect(ray, t_entry, t_exit, t_entryR, t_exitR);

				//isIntersectedLeft = true;
				//isIntersectedRight = true; 

				// -----------
				// Three cases
				// ----------

				// CASE 1 : two intervals are empty
				if (!isIntersectedLeft && !isIntersectedRight)
				{
				}

				// CASE 2 : Only one interval is not empty
				else if ((isIntersectedLeft && !isIntersectedRight) || (!isIntersectedLeft && isIntersectedRight))
				{
					// CASE LEFT SON
					if (isIntersectedLeft) i_found = left->traversingBVH(ray, t_entryL, t_exitL);
					// CASE RIGHT SON
					else if (isIntersectedRight) i_found = right->traversingBVH(ray, t_entryR, t_exitR);
				}

				// CASE 3 : The two intervals are not empty
				else
				{
					if (t_entryL < t_entryR)
					{
						i_found = left->traversingBVH(ray, t_entryL, t_exitL);
						if (!i_found.valid())
						{
							i_found = right->traversingBVH(ray, t_entryR, t_exitR);
						}
						else if (i_found.tRayValue() >= t_entryR)
						{
							i_found2 = right->traversingBVH(ray, t_entryR, i_found.tRayValue());
						}
					}
					else
					{
						i_found = right->traversingBVH(ray, t_entryR, t_exitR);
						if (!i_found.valid())
						{
							i_found = left->traversingBVH(ray, t_entryL, t_exitL);
						}
						else if (i_found.tRayValue() >= t_entryL)
						{
							i_found2 = left->traversingBVH(ray, t_entryL, i_found.tRayValue());
						}
					}



					if (i_found.valid())
					{
						if (i_found2.valid() && i_found2.tRayValue() < i_found.tRayValue())
						{
							return i_found2;
						}
						return i_found;
					}
					else
					{
						return i_found2;
					}
				}
			}


			return i_found;
		}

	};
}
