(define (problem ijrr_11)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street23 street24 - street
		building0 building13 building16 building33 - building
	)
	(:init
		(atLocation survivor0 street23)
		(atLocation survivor1 building13)
		(onFire building16)
		(onFire building33)
		(underRubble street24)
		(underRubble building0)
		(needsRepair building0)
		(needsRepair building16)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street24))
		(not (needsRepair building0))
		(not (needsRepair building16))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
